#include <globjects/base/baselogging.h>

#include <cassert>

#include <globjects/base/AbstractLogHandler.h>
#include <globjects/base/LogMessageBuilder.h>
#include <globjects/base/ConsoleLogger.h>

namespace
{
    globjects::LogMessage::Level l_verbosityLevel = globjects::LogMessage::Info;
    globjects::AbstractLogHandler * l_logHandler = new globjects::ConsoleLogger();
}

namespace globjects
{

LogMessageBuilder info(LogMessage::Level level)
{
    return LogMessageBuilder(level, level <= l_verbosityLevel ? l_logHandler : nullptr);
}

LogMessageBuilder debug()
{
    return info(LogMessage::Debug);
}

LogMessageBuilder warning()
{
    return info(LogMessage::Warning);
}

LogMessageBuilder critical()
{
    return info(LogMessage::Critical);
}

LogMessageBuilder fatal()
{
    return info(LogMessage::Fatal);
}

AbstractLogHandler * loggingHandler()
{
    return l_logHandler;
}

void setLoggingHandler(AbstractLogHandler* handler)
{
    delete l_logHandler;
    l_logHandler = handler;
}

void setVerbosityLevel(LogMessage::Level verbosity)
{
    l_verbosityLevel = verbosity;
}

LogMessage::Level verbosityLevel()
{
    return l_verbosityLevel;
}

} // namespace globjects
