#include "log.hpp"

/*
    logLevelMask allows to select log levels to output
*/
OstreamLogger::OstreamLogger(std::ostream &_os, u32 _logLevelMask)
    : os{_os}, logLevelMask{_logLevelMask} {}

void OstreamLogger::log(LogLevel level, const std::string &msg) {
    if(!((u32)level & logLevelMask)) return;
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
