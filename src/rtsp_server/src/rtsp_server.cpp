/*
 * Copyright (c) 2021 roleoroleo
 * Copyright (c) 2025 Noah Maceri
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <cstdint>
#include <fcntl.h>
#include <liveMedia.hh>
#include <BasicUsageEnvironment.hh>

#include <ServerMediaSubsession.h>
#include <record_audio.h>
#include <AlsaDeviceSource.h>

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

static int parse_ini(void *user, const char *section, const char *name, const char *value) {
    auto *config = static_cast<rtsp_settings *>(user);

    if (MATCH("rtsp", "username")) {
        config->user = strdup(value);
    } else if (MATCH("rtsp", "password")) {
        config->pwd = strdup(value);
    } else if (MATCH("rtsp", "port")) {
        config->port = strtoul(value, nullptr, 10);
    } else if (MATCH("rtsp", "name")) {
        config->name = strdup(value);
    } else if (MATCH("encoder", "height")) {
        config->resolution = strtoul(value, nullptr, 10);
    }

    return 1;
}

int main(int argc, char *argv[]) {
    // init zlog
    int rc;
    if ((rc = zlog_init("zlog.conf") < 0)) {
        fprintf(stderr, "Failed to initialize zlog: %d\n", rc);
        return rc;
    }

    zlog_category_t *c = zlog_get_category("server");

    zlog_info(c, "rRTSPServer v%d.%d.%d started", VER_MAJOR, VER_MINOR, VER_PATCH);

    rtsp_settings config;
    if (ini_parse("streamer.ini", parse_ini, &config) < 0) {
        zlog_fatal(c, "Failed to load streamer.ini");
        return EXIT_FAILURE;
    }

    // if the strings are empty strings set them to nullptr
    if (config.user && strcmp(config.user, "") == 0) {
        config.user = nullptr;
    }
    if (config.pwd && strcmp(config.pwd, "") == 0) {
        config.pwd = nullptr;
    }

    zlog_debug(c, "RTSP settings:");
    zlog_debug(c, "  Username: %s", config.user ? config.user : "None");
    zlog_debug(c, "  Password: %s", config.pwd ? config.pwd : "None");
    zlog_debug(c, "  Port: %u", config.port);
    zlog_debug(c, "  Stream Name: %s", config.name);

    // Begin by setting up our usage environment:
    TaskScheduler *scheduler = BasicTaskScheduler::createNew();
    BasicUsageEnvironment *env = BasicUsageEnvironment::createNew(*scheduler);

    UserAuthenticationDatabase *authDB = nullptr;
    if ((config.user != nullptr) && (config.pwd != nullptr)) {
        authDB = new UserAuthenticationDatabase;
        authDB->addUserRecord(config.user, config.pwd);
    }

    // Create the RTSP server:
    RTSPServer *rtspServer = RTSPServer::createNew(*env, config.port, authDB);
    if (rtspServer == nullptr) {
        zlog_fatal(c, "Failed to create RTSP server: %s", env->getResultMsg());
        exit(EXIT_FAILURE);
    }

    AlsaDeviceSource *es = nullptr;
    audio *ac = audio_new(nullptr, nullptr);
    if (ac) {
        zlog_info(c, "Audio source created");
        if((rc = audio_start(ac))) {
            zlog_fatal(c, "Failed to start audio source: %s", strerror(rc));
        }
        zlog_info(c, "Audio source started");
        es = AlsaDeviceSource::createNew(*env, -1, 10, true);
        if (es == nullptr) {
            zlog_fatal(c, "Failed to create audio dev_src \n");
        } else {
            ac->dev_src = es;
        }
        zlog_info(c, "Audio dev_src created");
    } else {
        zlog_fatal(c, "Failed to create audio source: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    StreamReplicator *audio_replicator = nullptr;
    audio_replicator = StreamReplicator::createNew(*env, es, false);
    if (audio_replicator == nullptr) {
        zlog_fatal(c, "Failed to create audio replicator: %s", env->getResultMsg());
        exit(EXIT_FAILURE);
    }
    zlog_info(c, "Audio replicator created");

    OutPacketBuffer::maxSize = 300000;
    ServerMediaSession *sms = ServerMediaSession::createNew(*env, config.name, "", "");
    sms->addSubsession(H264VideoFileServerMediaSubsession::createNew(*env, VIDEO_FIFO, True));
    ServerMediaSubsession *sms_aud = UnicastServerMediaSubsession::createNew(*env, audio_replicator);
    sms->addSubsession(sms_aud);
    rtspServer->addServerMediaSession(sms);
    zlog_info(c, "ServerMediaSession added");
    zlog_info(c, "RTSP server is running on %s", rtspServer->rtspURL(sms));
    env->taskScheduler().doEventLoop(); // does not return

    if ((rc = audio_stop(ac))) {
        zlog_fatal(c, "Failed to stop audio source: %s", strerror(rc));
    }
    audio_free(ac);
    ac = nullptr;

    return EXIT_SUCCESS;
}
