
#include "Daemon.h"

const char* Daemon::s_LockFilePath = "/var/lock/matt_daemon.lock";
const uint16_t Daemon::s_port = 4242;
const size_t Daemon::s_max_size = 1024;

void Daemon::SignalsHandler(void) {

}

void Daemon::CloseAllFD(void) {

    struct rlimit	limit;
    int             iteri = -1;
    int             fd0;

    if (getrlimit(RLIMIT_NOFILE, &limit) < 0)
    {
        std::cout <<"Initial getrlimit failed." << std::endl;
        exit(EXIT_FAILURE);
    }
    if (limit.rlim_max == RLIM_INFINITY)
        limit.rlim_max = MAX_FD;
    while (++iteri < static_cast<int>(limit.rlim_max))
        close(iteri);
    if ((fd0 = open("/dev/null", O_RDWR)) < 0)
    {
        std::cout << "Can't open: /dev/null;" << std::endl;
        exit(EXIT_FAILURE);
    }
    dup(fd0);
    dup(fd0);
}

void Daemon::CreateServer(void) {
    struct protoent *proto;
    struct sockaddr_in sin;
    int optval;

    m_Loger->log(INFO, std::string("Creating server."));
    if (!(proto = getprotobyname("tcp"))) {
        m_Loger->log(ERROR, std::string("The getprotobyname failure."));
        exit(EXIT_FAILURE);
    }
    if ((m_socket = socket(PF_INET, SOCK_STREAM, proto->p_proto)) < 0) {
        m_Loger->log(ERROR, std::string("The socket failure."));
        exit(EXIT_FAILURE);
    }
    sin.sin_family = AF_INET;
    sin.sin_port = htons(s_port);
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    optval = 1;
    if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) == -1) {
        m_Loger->log(ERROR, std::string("Set the socket options failure."));
        exit(EXIT_FAILURE);
    }
    if (bind(m_socket, (const struct sockaddr *) &sin, sizeof(sin)) == -1) {
        m_Loger->log(ERROR, std::string("Binding failure."));
        exit(EXIT_FAILURE);
    }
    if (listen(m_socket, MAX_CLIENT) == -1) {
        m_Loger->log(ERROR, std::string("Listening failure."));
        exit(EXIT_FAILURE);
    }
    m_CountClient = m_socket;
    m_Loger->log(INFO, std::string("Server created."));
}

void Daemon::DaemonMode(void) {
    pid_t pid;

    m_Loger->log(INFO, std::string("Entering Daemon mode."));
    if ((pid = fork()) < 0) {
        m_Loger->log(ERROR, std::string("Initial fork failed;"));
        exit(EXIT_FAILURE);
    }
    else if (pid != 0) {
        m_Loger->log(INFO, std::string("Started. PID: ") + std::to_string(pid));
        exit(EXIT_SUCCESS);
    }
    umask(0);
    if (chdir("/") < 0) {
        m_Loger->log(ERROR, std::string("It is not possible to make the current working directory /"));
        exit(EXIT_FAILURE);
    }
    if (setsid() == -1) {
        m_Loger->log(ERROR, std::string("Initial setsid failed;"));
        exit(EXIT_FAILURE);
    }
}

void Daemon::Reset(void) {

    FD_ZERO(&m_ReadFD);
    FD_SET(m_socket, &m_ReadFD);
    for (int i = 0; i < MAX_CLIENT; i++)
    {
        if(m_CliFd[i] != 0)
        {
            FD_SET(m_CliFd[i], &m_ReadFD);
        }
    }
    if (select(m_CountClient + 1, &m_ReadFD, NULL, NULL, NULL) == -1)
    {
        m_Loger->log(ERROR, std::string("The select is failure."));
        exit(EXIT_FAILURE);
    }
}

void Daemon::AcceptClient(void) {
    struct sockaddr_in  cli_sin;
    socklen_t           size;

    size = sizeof(cli_sin);
    for (int i = 0; i < MAX_CLIENT; i++)
    {
        if (m_CliFd[i] == 0)
        {
            if ((m_CliFd[i] = accept(m_socket, (struct sockaddr *)&cli_sin, &size)) == -1)
            {
                m_Loger->log(ERROR, std::string("Connection refused."));
                m_CliFd[i] = 0;
                return ;
            }
            m_CountClient++;
            m_Loger->log(INFO, std::string("Connection accepted."));
            return ;
        }
    }
    m_Loger->log(ERROR, std::string("No room from new client."));
    close(accept(m_socket, (struct sockaddr *)&cli_sin, &size));
}

void Daemon::AcceptFromClient(int index) {
    char    buff[s_max_size];
    ssize_t size;

    std::memset(buff, 0, s_max_size);
    if ((size = recv(m_CliFd[index], &buff, s_max_size - 1, 0)) <= 0)
    {
        close(m_CliFd[index]);
        m_CliFd[index] = 0;
        m_CountClient--;
        m_Loger->log(INFO, "Client disconnect.");
        return ;
    }
    buff[size] = '\0';
    if (std::strcmp(buff, "quit\n") != 0) {
        m_Loger->log(LOG, std::string(buff));
    } else {
        m_Loger->log(INFO, std::string("Request quit."));
        m_status_running = false;
    }
}

bool Daemon::CheckLockAndLocked(void) {

    struct flock    fl;

    if ((m_LockFd = open(s_LockFilePath, O_RDWR | O_CREAT, LOCKMODE)) < 0) {
        return false;
    }
    fl.l_type = F_WRLCK;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;

    if (fcntl(m_LockFd, F_SETLKW, &fl) < 0)
    {
        if (errno == EACCES || errno == EAGAIN)
        {
            close(m_LockFd);
            m_Loger->log(ERROR, std::string("Error file locked."));
            return false;
        }
        return false;
    }
    return true;
}

Daemon::Daemon(void) {

    if (getuid() != 0)
    {
        std::cout << "Permission denied. " << std::endl;//throw
        exit(EXIT_FAILURE);
    }
    CloseAllFD();
    if (!(m_Loger = new Tintin_reporter()))
    {
        std::cout << "Error: FAIL to allocate memory for Tintin_reporter." << std::endl;//throw
        exit(EXIT_FAILURE);
    }
    if (!CheckLockAndLocked()) {/*throw (*/
        std::cout << "Can't open :/var/lock/matt_daemon.lock" << std::endl;
        exit(EXIT_FAILURE);
    } else
    {
        write(m_LockFd, "FACK", 4);
    }
    m_Loger->log(INFO, std::string("Started."));
    SignalsHandler();
    CreateServer();
    DaemonMode();
    std::memset(m_CliFd, 0, MAX_CLIENT * sizeof(int));
    m_status_running = false;
}

Daemon::Daemon(Daemon const &src) {
    *this = src;
}

Daemon& Daemon::operator=( Daemon const &rhs ) {
    if (this != &rhs) {
        m_socket = rhs.m_socket;
        m_ReadFD = rhs.m_ReadFD;
        m_Loger = rhs.m_Loger;
        m_status_running = rhs.m_status_running;
        for (int i = 0; i  < MAX_CLIENT; i++)
            m_CliFd[i] = rhs.m_CliFd[i];
        m_CountClient = rhs.m_CountClient;
    }
    return *this;
}

Daemon::~Daemon() {
    close(m_socket);
    close(m_LockFd);
    for (int i = 0; i < MAX_CLIENT; i++)
    {
        if (m_CliFd[i] != 0)
            close(m_CliFd[i]);
    }
    m_Loger->log(INFO, std::string("Quitting."));
    if (m_Loger)
        delete m_Loger;
}

void Daemon::Run(void) {

    m_status_running = true;
    while (m_status_running) {
        Reset();
        if (FD_ISSET(m_socket, &m_ReadFD))
            AcceptClient();
        for (int i = 0; i < MAX_CLIENT; i++) {
            if (FD_ISSET(m_CliFd[i], &m_ReadFD))
            {
                AcceptFromClient(i);
            }
        }
    }
}
