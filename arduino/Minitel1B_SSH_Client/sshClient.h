// source: https://github.com/jbellue/3615_SSH

#ifndef _SSH_CLIENT_H
#define _SSH_CLIENT_H

#include "libssh_esp32.h"
#include <libssh/libssh.h>
#include <string.h>

//#include <Minitel1B_Hard.h>

class SSHClient {
    public:
    SSHClient();

    enum class SSHStatus {
        OK,
        AUTHENTICATION_ERROR,
        GENERAL_ERROR
    };

    bool begin(const char *host, const char *user, const char *password);
    SSHStatus connect_ssh(const char *host, const char *user, const char *password, const int verbosity);
//    bool poll(Minitel* minitel);
    SSHStatus start_session(const char *host, const char *user, const char *password);
    void close_session();
    int interactive_shell_session();
    void close_channel();
    bool open_channel();
    void end();
    bool available();
    int receive();
    int flushReceiving();
    char readIndex(int index);
    int send(void *buffer, uint32_t len);

    private:
    ssh_session _session;
    ssh_channel _channel;
    char _readBuffer[256] = {'\0'};
//    size_t getMinitelInput(unsigned long key, char* buffer);
};

#endif
