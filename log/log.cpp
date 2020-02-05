#include "log.hpp"

OstreamLogger::OstreamLogger(std::ostream &_os)
    : os{_os} {}

void OstreamLogger::log(LogLevel level, const std::string &msg) {
    switch(level) {
    case LogLevel::Debug:
        os << "DEBUG: " << msg << std::endl;
        break;
    case LogLevel::Info:
        os << "INFO: " << msg << std::endl;
        break;
    case LogLevel::Warning:
        os << "WARNING: " << msg << std::endl;
        break;
    case LogLevel::Error:
        os << "ERROR: " << msg << std::endl;
        break;
    default:
        throw std::runtime_error("OstreamLogger::log() - unknown log level");
    }
}
