
#ifndef MATT_DAEMON_DAEMON_H
#define MATT_DAEMON_DAEMON_H

#include <unistd.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include "Tintin_reporter.h"
#include <cstdlib>
#include <errno.h>
#include <csignal>

#define LOCKMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
#define MAX_FD 1024
#define MAX_CLIENT 3

class Daemon {
private:
    static const char*      s_LockFilePath;
    static const uint16_t   s_port;
    static const size_t     s_max_size;
    Tintin_reporter         *m_Loger;
    int                     m_LockFd;
    int                     m_socket;
    int                     m_CliFd[MAX_CLIENT];
    fd_set                  m_ReadFD;
    int                     m_CountClient;
    struct timeval          m_TimeOut;
    bool                    m_status_running;

    void    CloseAllFD(void);
    void    CreateServer(void);
    void    DaemonMode(void);
    void    Reset(void);
    void    AcceptClient(void);
    void    AcceptFromClient(int index);
    bool    CheckLockAndLocked(void);

public:
    Daemon(void);
    Daemon( Daemon const &src );
    Daemon	&operator=( Daemon const &rhs );
    ~Daemon(void);
    void Run(void);
    void SignalsHandler(int signal);
};

#endif //MATT_DAEMON_DAEMON_H
