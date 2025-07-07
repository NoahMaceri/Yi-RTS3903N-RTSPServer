#ifndef RTSP_SERVER_H
#define RTSP_SERVER_H

#include <stdint.h>
#include <liveMedia.hh>
#include <BasicUsageEnvironment.hh>
#include <zlog.h>
#include <ini.h>
#include <ver.h>
#include <globals.h>

typedef struct {
    const char* user;
    const char* pwd;
    uint16_t port;
    const char* name;
    uint16_t resolution;
} rtsp_settings;

#endif //RTSP_SERVER_H
