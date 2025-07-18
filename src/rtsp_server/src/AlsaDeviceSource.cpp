#include "AlsaDeviceSource.h"

/**
 * @brief Tracks and logs statistics for audio frames
 *
 * @param tv_sec Current time in seconds
 * @param frame_sz Size of the current frame in bytes
 * @return int The current frame count
 */
int AlsaDeviceSource::Stats::notify(const int tv_sec, const int frame_sz) {
	m_fps++;
	m_size += frame_sz;
	if (tv_sec != m_fps_sec) {
		zlog_debug(zlog_get_category(ALSA_LOG), "audio %s tv_sec: %d, framecount: %d, bitrate: %d kbps", m_msg.c_str(), tv_sec, m_fps, (m_size / 128));
		m_fps_sec = tv_sec;
		m_fps = 0;
		m_size = 0;
	}
	return m_fps;
}

/**
 * @brief Factory method to create a new AlsaDeviceSource instance
 *
 * @param env The usage environment for the Live555 framework
 * @param outputFd File descriptor for optional output (use -1 to disable)
 * @param queueSize Maximum size of the internal frame queue
 * @param useThread Whether to use a separate thread for processing
 * @return AlsaDeviceSource* Pointer to newly created AlsaDeviceSource or NULL if creation failed
 */
AlsaDeviceSource *AlsaDeviceSource::createNew(UsageEnvironment &env, const int outputFd, const unsigned int queueSize, const bool useThread) {
	AlsaDeviceSource *source = nullptr;
	source = new AlsaDeviceSource(env, outputFd, queueSize, useThread);
	return source;
}

/**
 * @brief Constructor for AlsaDeviceSource
 *
 * @param env The usage environment for the Live555 framework
 * @param outputFd File descriptor for optional output (use -1 to disable)
 * @param queueSize Maximum size of the internal frame queue
 * @param useThread Whether to use a separate thread for processing
 */
AlsaDeviceSource::AlsaDeviceSource(UsageEnvironment &env, const int outputFd, const unsigned int queueSize, bool useThread)
	: FramedSource(env),
	  m_in("in"),
	  m_out("out"),
	  m_out_file(outputFd),
	  m_queueSize(queueSize) {
	m_eventTriggerId = envir().taskScheduler().createEventTrigger(AlsaDeviceSource::deliverFrameStub);
	memset(&m_mutex, 0, sizeof(m_mutex));
	pthread_mutex_init(&m_mutex, nullptr);
}

/**
 * @brief Destructor for AlsaDeviceSource
 * Cleans up resources and destroys mutex
 */
AlsaDeviceSource::~AlsaDeviceSource() {
	envir().taskScheduler().deleteEventTrigger(m_eventTriggerId);
	pthread_mutex_destroy(&m_mutex);
}

/**
 * @brief Main thread function for asynchronous processing
 *
 * @return void* Return value of the thread (always NULL)
 */
void *AlsaDeviceSource::thread() {
	return nullptr;
}

/**
 * @brief Callback for Live555 framework when it's ready for a new frame
 * This function is called when the sink is ready to process a new frame
 */
void AlsaDeviceSource::doGetNextFrame() {
	int isQueueEmpty = 0;
	//printf("AlsaDeviceSource::doGetNextFrame\n");
	pthread_mutex_lock(&m_mutex);
	isQueueEmpty = m_captureQueue.empty();
	pthread_mutex_unlock(&m_mutex);

	if (!isQueueEmpty)
		deliverFrame();
}

/**
 * @brief Stops the delivery of frames
 * Called by Live555 framework when streaming should stop
 */
void AlsaDeviceSource::doStopGettingFrames() {
	FramedSource::doStopGettingFrames();
}

/**
 * @brief Delivers a frame from the queue to the consumer
 * Processes the next frame in the queue and passes it to the Live555 framework
 */
void AlsaDeviceSource::deliverFrame() {
	if (isCurrentlyAwaitingData()) {
		int isQueueEmpty = 0;
		fDurationInMicroseconds = 0;
		fFrameSize = 0;

		pthread_mutex_lock(&m_mutex);
		isQueueEmpty = m_captureQueue.empty();
		pthread_mutex_unlock(&m_mutex);

		if (!isQueueEmpty) {
			gettimeofday(&fPresentationTime, nullptr);

			pthread_mutex_lock(&m_mutex);

			Frame *frame = m_captureQueue.front();
			m_captureQueue.pop_front();

			pthread_mutex_unlock(&m_mutex);

			m_out.notify(fPresentationTime.tv_sec, frame->m_size);
			if (frame->m_size > fMaxSize) {
				fFrameSize = fMaxSize;
				fNumTruncatedBytes = frame->m_size - fMaxSize;
			} else {
				fFrameSize = frame->m_size;
			}
			timeval diff{};
			timersub(&fPresentationTime, &(frame->m_timestamp), &diff);

			memcpy(fTo, frame->m_buffer, fFrameSize);

			delete frame;
		}

		// send Frame to the consumer
		FramedSource::afterGetting(this);
	}
}

/**
 * @brief Handles frame reading events from the source
 *
 * @return int Status code (0 for success)
 */
int AlsaDeviceSource::getNextFrame() {
	return 0;
}

/**
 * @brief Callback function for audio data
 * This function is called when new audio data is available
 *
 * @param tv Timestamp of the audio data
 * @param data Pointer to the audio data buffer
 * @param len Length of the audio data in bytes
 * @param cb_args Additional callback arguments
 */
void AlsaDeviceSource::audio_cb(const struct timeval *tv, void *data, const size_t len, void *cb_args) {
	if (data == nullptr || len == 0) {
		zlog_fatal(zlog_get_category(ALSA_LOG), "Buffer is null or length is zero");
		return;
	}
	processFrame(static_cast<char *>(data), static_cast<int>(len), *tv);
}

/**
 * @brief Processes a frame of audio data
 * Splits the frame if necessary and queues it for delivery
 *
 * @param frame Pointer to the frame data
 * @param frame_sz Size of the frame in bytes
 * @param ref Reference timestamp for the frame
 */
void AlsaDeviceSource::processFrame(char *frame, const int frame_sz, const timeval &ref) {
	timeval tv{};
	gettimeofday(&tv, nullptr);
	timeval diff{};
	timersub(&tv, &ref, &diff);

	std::list<std::pair<unsigned char *, size_t> > frameList = this->splitFrames(reinterpret_cast<unsigned char *>(frame), frame_sz);
	while (!frameList.empty()) {
		const std::pair<unsigned char *, size_t> &tmp_frame = frameList.front();
		const size_t size = tmp_frame.second;
		const auto buf = new char[size];
		memcpy(buf, tmp_frame.first, size);
		queueFrame(buf, static_cast<int>(size), ref);

		if (m_out_file != -1)
			write(m_out_file, buf, size);

		frameList.pop_front();
	}
}

/**
 * @brief Adds a frame to the internal queue
 * Manages queue size and triggers event for frame delivery
 *
 * @param frame Pointer to the frame data
 * @param frameSize Size of the frame in bytes
 * @param tv Timestamp of the frame
 */
void AlsaDeviceSource::queueFrame(char *frame, int frameSize, const timeval &tv) {
	pthread_mutex_lock(&m_mutex);
	while (m_captureQueue.size() >= m_queueSize) {
		delete m_captureQueue.front();
		m_captureQueue.pop_front();
	}
	m_captureQueue.push_back(new Frame(frame, frameSize, tv));
	pthread_mutex_unlock(&m_mutex);

	envir().taskScheduler().triggerEvent(m_eventTriggerId, this);
}

/**
 * @brief Splits a packet into multiple frames if needed
 *
 * @param frame Pointer to the packet data
 * @param frameSize Size of the packet in bytes
 * @return std::list<std::pair<unsigned char *, size_t>> List of frame data and size pairs
 */
std::list<std::pair<unsigned char *, size_t> > AlsaDeviceSource::splitFrames(unsigned char *frame, unsigned frameSize) {
	std::list<std::pair<unsigned char *, size_t> > frameList;
	if (frame != nullptr) {
		frameList.push_back(std::make_pair<unsigned char *, size_t>(std::move(frame), std::move(frameSize)));
	}
	return frameList;
}
