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
#include <sn98600_record_audio.h>
#include <ServerMediaSubsession.h>
#include <AlsaDeviceSource.h>

void listdev(char *devname) {
    char **hints;
    int err;
    char **n;
    char *name;
    char *desc;
    char *ioid;

    /* Enumerate sound devices */
    err = snd_device_name_hint(-1, devname, (void ***) &hints);
    if (err != 0) {
        fprintf(stderr, "*** Cannot get device names\n");
        exit(1);
    }

    n = hints;
    while (*n != NULL) {
        name = snd_device_name_get_hint(*n, "NAME");
        desc = snd_device_name_get_hint(*n, "DESC");
        ioid = snd_device_name_get_hint(*n, "IOID");

        printf("Name of device: %s\n", name);
        printf("Description of device: %s\n", desc);
        printf("I/O type of device: %s\n", ioid);
        printf("\n");

        if (name && strcmp("null", name)) free(name);
        if (desc && strcmp("null", desc)) free(desc);
        if (ioid && strcmp("null", ioid)) free(ioid);
        n++;
    }

    //Free hint buffer too
    snd_device_name_free_hint((void **) hints);
}

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

void closeAudioCapure(struct sonix_audio *m_fd) {
    int rc;
    if (m_fd) {
        if ((rc = snx98600_record_audio_stop(m_fd))) {
            fprintf(stderr, "failed to start audio source: %s\n", strerror(rc));
        }

        if (m_fd) {
            snx98600_record_audio_free(m_fd);
            m_fd = NULL;
        }
    }
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

    int rc;
    AlsaDeviceSource *audioES = NULL;
    struct sonix_audio *audioCapture = snx98600_record_audio_new(AUDIO_RECORD_DEV, nullptr, nullptr);
    if (!audioCapture) {
        rc = errno ? errno : -1;
        fprintf(stderr, "failed to create audio source: %s\n", strerror(rc));
    }
    fprintf(stderr, "Audio capture created: %s\n", audioCapture ? "Success" : "Failed");
    if (audioCapture) {
        if ((rc = snx98600_record_audio_start(audioCapture))) {
            fprintf(stderr, "failed to start audio source: %s\n", strerror(rc));
        }
    }
    fprintf(stderr, "Audio capture started: %s\n", audioCapture ? "Success" : "Failed");
    if (audioCapture) {
        audioES = AlsaDeviceSource::createNew(*env, -1, 10, true);
        if (audioES == NULL) {
            fprintf(stderr, "Unable to create audio devicesource \n");
        } else {
            audioCapture->devicesource = audioES;
        }
    }
    fprintf(stderr, "Audio devicesource created: %s\n", audioES ? "Success" : "Failed");
    StreamReplicator *audio_replicator = nullptr;
    audio_replicator = StreamReplicator::createNew(*env, audioES, false);
    fprintf(stderr, "Audio replicator created: %s\n", audio_replicator ? "Success" : "Failed");

    OutPacketBuffer::maxSize = 300000;
    ServerMediaSession *sms = ServerMediaSession::createNew(*env, config.name, "", "");
    sms->addSubsession(H264VideoFileServerMediaSubsession::createNew(*env, VIDEO_FIFO, True));
    ServerMediaSubsession *sms_aud = UnicastServerMediaSubsession::createNew(*env, audio_replicator);
    sms->addSubsession(sms_aud);
    rtspServer->addServerMediaSession(sms);

    fprintf(stderr, "ServerMediaSession added to RTSP server: %s\n", sms ? "Success" : "Failed");
    zlog_info(c, "RTSP server is running on %s", rtspServer->rtspURL(sms));
    env->taskScheduler().doEventLoop(); // does not return

    return 0;
}
