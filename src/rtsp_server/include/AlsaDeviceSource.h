/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** AlsaDeviceSource.h
**
** -------------------------------------------------------------------------*/
#ifndef ALSA_DEVICE_SOURCE
#define ALSA_DEVICE_SOURCE

#include <string>
#include <list>
#include <iomanip>
#include <utility>
#include <sstream>

#include <liveMedia.hh>
#include <zlog.h>

#include "audio_defines.h"

class AlsaDeviceSource final : public FramedSource {
public:
	class Stats {
		public:
			explicit Stats(std::string msg) : m_fps(0), m_fps_sec(0), m_size(0), m_msg(std::move(msg)) {};
			int notify(int tv_sec, int frame_sz);
		protected:
			int m_fps;
			int m_fps_sec;
			int m_size;
			const std::string m_msg;
	};

	struct Frame {
		Frame(char *buffer, const int size, const timeval timestamp) : m_buffer(buffer), m_size(size), m_timestamp(timestamp) {};
		Frame(const Frame &);
		Frame &operator=(const Frame &);
		~Frame() { delete m_buffer; };

		char *m_buffer;
		int m_size;
		timeval m_timestamp;
	};

	// Public functions
	static AlsaDeviceSource *createNew(UsageEnvironment &env, int outputFd, unsigned int queueSize, bool useThread);
	std::string getAuxLine() { return m_auxLine; };
	void audio_cb(const timeval *tv, void *data, size_t len, void *cb_args);

protected:
	// Protected functions
	AlsaDeviceSource(UsageEnvironment &env, int outputFd, unsigned int queueSize, bool useThread);
	~AlsaDeviceSource() override;
	static void *threadStub(void *clientData) { return AlsaDeviceSource::thread(); };
	static void *thread();
	static void deliverFrameStub(void *clientData) { static_cast<AlsaDeviceSource *>(clientData)->deliverFrame(); };
	void deliverFrame();
	static void incomingPacketHandlerStub(void *clientData, int mask) { AlsaDeviceSource::getNextFrame(); };
	static int getNextFrame();
	void processFrame(char *frame, int frame_sz, const timeval &ref);
	void queueFrame(char *frame, int frameSize, const timeval &tv);
	std::list<std::pair<unsigned char *, size_t> > splitFrames(unsigned char *frame, unsigned frameSize);
	void doGetNextFrame() override;
	void doStopGettingFrames() override;

	// Protected members
	std::list<Frame *> m_captureQueue;
	Stats m_in;
	Stats m_out;
	EventTriggerId m_eventTriggerId;
	int m_out_file;
	unsigned int m_queueSize;
	pthread_t m_thread{};
	pthread_mutex_t m_mutex{};
	std::string m_auxLine;
};

#endif
