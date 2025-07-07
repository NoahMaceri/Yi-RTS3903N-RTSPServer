/*
 * Copyright (c) 2021 roleo.
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

#include <rtsp_server.h>

static int parse_ini(void* user, const char* section, const char* name, const char* value) {
    auto* config = static_cast<rtsp_settings *>(user);

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
    if (zlog_init("zlog.conf") < 0) {
        fprintf(stderr, "Failed to initialize zlog\n");
        return EXIT_FAILURE;
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
    UsageEnvironment *env = BasicUsageEnvironment::createNew(*scheduler);

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

    // First, make sure that the RTPSinks' buffers will be large enough to handle the huge size of DV frames (as big as 288000).
    OutPacketBuffer::maxSize = 300000;

    ServerMediaSession *sms= ServerMediaSession::createNew(*env, config.name, config.name, "RTSP server");
    sms->addSubsession(H264VideoFileServerMediaSubsession::createNew(*env, VIDEO_SINK, True));
    // sms->addSubsession(MP3AudioFileServerMediaSubsession::createNew(*env, AUDIO_SINK, True, True, nullptr));
    rtspServer->addServerMediaSession(sms);
    env->taskScheduler().doEventLoop(); // does not return

    return 0;
}
