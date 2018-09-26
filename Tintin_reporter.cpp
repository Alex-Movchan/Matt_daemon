
#include "Tintin_reporter.h"

const char* Tintin_reporter::s_logFilePath = "/var/log/matt_daemon/matt_daemon.log";

Tintin_reporter::Tintin_reporter(void) {
    m_LogFile.open(s_logFilePath, std::ios::out | std::ios::app);
    if (!m_LogFile.is_open())
        /*throw*/std::cout << "Error: opening /var/log/matt_daemon/matt_daemon.log;" << std::endl;
}

Tintin_reporter::Tintin_reporter( Tintin_reporter const &src ) {
    *this = src;
}

Tintin_reporter& Tintin_reporter::operator=( Tintin_reporter const &rhs ) {
    if(this != &rhs) {
        m_LogFile.copyfmt(rhs.m_LogFile);
        m_LogFile.clear(rhs.m_LogFile.rdstate());
        m_LogFile.basic_ios<char>::rdbuf(rhs.m_LogFile.rdbuf());
    }
    return *this;
}

std::string Tintin_reporter:: getCurrentTime( void ) {
    std::time_t	now;
    char 		strtime[65];

    now = std::time(NULL);
    std::strftime(strtime, sizeof(strtime), "[%d/%m/%Y-%H:%M:%S]", std::localtime(&now));

    return(std::string(strtime));
}

void Tintin_reporter::log(LogType type, std::string msg) {
    if (!msg.empty() && msg[msg.length() - 1] == '\n')
        msg.erase(msg.length() - 1);
    switch (type) {
        case INFO:
            m_LogFile << getCurrentTime() << " [ INFO ] - Matt_daemon: "<< msg << std::endl;
            break ;
        case ERROR:
            m_LogFile << getCurrentTime() << " [ ERROR ] - Matt_daemon: "<< msg << std::endl;
            break ;
        case LOG:
            m_LogFile << getCurrentTime() << " [ LOG ] - Matt_daemon: User input: "<< msg << std::endl;
            break ;
        default:
            break ;
    }
}

Tintin_reporter::~Tintin_reporter() {
    m_LogFile.close();
}