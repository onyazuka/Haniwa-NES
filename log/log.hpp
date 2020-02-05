#pragma once
#include <string>
#include <ostream>

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error
};

class Logger {
public:
    virtual void log(LogLevel level, const std::string& msg) = 0;
};

class OstreamLogger : public Logger {
public:
    OstreamLogger(std::ostream& _os);
    void log(LogLevel level, const std::string& msg);
private:
    std::ostream& os;
};
