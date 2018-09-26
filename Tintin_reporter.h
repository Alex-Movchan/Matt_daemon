

#ifndef MATT_DAEMON_TINTIN_REPORTER_H
#define MATT_DAEMON_TINTIN_REPORTER_H

#include <string>
#include <ostream>
#include <ctime>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>

enum LogType {
    INFO,
    ERROR,
    LOG
};


class Tintin_reporter {

private:
    static const char*  s_logFilePath;
    std::ofstream        m_LogFile;
    std::string         getCurrentTime(void);

public:
    Tintin_reporter(void);
    ~Tintin_reporter(void);
    Tintin_reporter(Tintin_reporter const &src);
    Tintin_reporter	&operator=(Tintin_reporter const &rhs);
    void    log(LogType type, std::string msg);

};


#endif //MATT_DAEMON_TINTIN_REPORTER_H
