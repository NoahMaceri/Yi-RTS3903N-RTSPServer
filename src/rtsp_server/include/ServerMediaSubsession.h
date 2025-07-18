/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** ServerMediaSubsession.h
** 
** -------------------------------------------------------------------------*/

#ifndef SERVER_MEDIA_SUBSESSION
#define SERVER_MEDIA_SUBSESSION

#include <liveMedia.hh>
#include <zlog.h>

#include "audio_defines.h"

// ---------------------------------
//   BaseServerMediaSubsession
// ---------------------------------
class BaseServerMediaSubsession {
public:
	explicit BaseServerMediaSubsession(StreamReplicator *replicator): m_replicator(replicator) {};
	static FramedSource *createSource(UsageEnvironment &env, FramedSource *es);
	RTPSink *createSink(UsageEnvironment &env, Groupsock *rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic) const;
protected:
	StreamReplicator *m_replicator;
};

// -----------------------------------------
//    ServerMediaSubsession for Unicast
// -----------------------------------------
class UnicastServerMediaSubsession : public OnDemandServerMediaSubsession, public BaseServerMediaSubsession {
public:
	static UnicastServerMediaSubsession *createNew(UsageEnvironment &env, StreamReplicator *replicator);
protected:
	UnicastServerMediaSubsession(UsageEnvironment &env, StreamReplicator *replicator) : OnDemandServerMediaSubsession(env, False), BaseServerMediaSubsession(replicator) {};
	FramedSource *createNewStreamSource(unsigned clientSessionId, unsigned &estBitrate) override;
	RTPSink *createNewRTPSink(Groupsock *rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource *inputSource) override;
};

#endif
