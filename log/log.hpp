#pragma once
#include <string>
#include <ostream>

typedef uint32_t u32;

enum class LogLevel {
    Debug = 1,
    Info = 2,
    Warning = 4,
    Error = 8
};

class Logger {
public:
    virtual ~Logger() {}
    virtual void log(LogLevel level, const std::string& msg) = 0;
};

class OstreamLogger : public Logger {
public:
    OstreamLogger(std::ostream& _os, u32 logLevelMask);
    void log(LogLevel level, const std::string& msg);
private:
    std::ostream& os;
    u32 logLevelMask;
};




