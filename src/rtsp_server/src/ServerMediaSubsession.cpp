#include "ServerMediaSubsession.h"

FramedSource *BaseServerMediaSubsession::createSource(UsageEnvironment &env, FramedSource *es) {
	return es;
}

RTPSink *BaseServerMediaSubsession::createSink(UsageEnvironment &env, Groupsock *rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic) const {
	zlog_info(zlog_get_category(SMSS_LOG), "Creating audio sink with payload type %d, sample rate %d, MIME type %s, channels %d", RTSP_PAYLOAD_TYPE, SAMPLE_RATE, RTSP_MIME_TYPE, CHANNELS);
	return SimpleRTPSink::createNew(env, rtpGroupsock, RTSP_PAYLOAD_TYPE, SAMPLE_RATE, "audio", RTSP_MIME_TYPE, CHANNELS);
}

// -----------------------------------------
//    ServerMediaSubsession for Unicast
// -----------------------------------------
UnicastServerMediaSubsession *UnicastServerMediaSubsession::createNew(UsageEnvironment &env, StreamReplicator *replicator) {
	return new UnicastServerMediaSubsession(env, replicator);
}

FramedSource *UnicastServerMediaSubsession::createNewStreamSource(unsigned clientSessionId, unsigned &estBitrate) {
	FramedSource *source = m_replicator->createStreamReplica();
	return createSource(envir(), source);
}


RTPSink *UnicastServerMediaSubsession::createNewRTPSink(Groupsock *rtpGroupsock, const unsigned char rtpPayloadTypeIfDynamic, FramedSource *inputSource) {
	return createSink(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
}
