/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** ServerMediaSubsession.cpp
** 
** -------------------------------------------------------------------------*/

#include <sstream>

// libv4l2
#include <linux/videodev2.h>

// live555
#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>
#include <Base64.hh>
#include "JPEGVideoRTPSink.hh"

// project
#include "ServerMediaSubsession.h"


// ---------------------------------
//   BaseServerMediaSubsession
// ---------------------------------
FramedSource* BaseServerMediaSubsession::createSource(UsageEnvironment& env, FramedSource * videoES)
{
	return videoES;
}

RTPSink*  BaseServerMediaSubsession::createSink(UsageEnvironment& env, Groupsock * rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic)
{
	char const* mimeType = "L16";
	unsigned char payloadFormatCode = 97;
	unsigned const samplingFrequency = SAMPLE_RATE;
	unsigned char const numChannels = 1;
	fprintf(stderr, "create audio sink %d %d %s \n",  payloadFormatCode, samplingFrequency, mimeType);
	return SimpleRTPSink::createNew(env, rtpGroupsock, payloadFormatCode, samplingFrequency, "audio", mimeType, numChannels);
}

// -----------------------------------------
//    ServerMediaSubsession for Unicast
// -----------------------------------------
UnicastServerMediaSubsession* UnicastServerMediaSubsession::createNew(UsageEnvironment& env, StreamReplicator* replicator)
{ 
	return new UnicastServerMediaSubsession(env,replicator);
}
					
FramedSource* UnicastServerMediaSubsession::createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate)
{
	FramedSource* source = m_replicator->createStreamReplica();
	return createSource(envir(), source);
}

		
RTPSink* UnicastServerMediaSubsession::createNewRTPSink(Groupsock* rtpGroupsock,  unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource)
{
	return createSink(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
}
