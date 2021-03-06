#include <globjects/DebugMessage.h>

#include <sstream>
#include <cassert>

#include <glbinding/gl/enum.h>
#include <glbinding/gl/boolean.h>

#include "registry/Registry.h"
#include "registry/ImplementationRegistry.h"
#include "implementations/AbstractDebugImplementation.h"


using namespace gl;

namespace globjects 
{

AbstractDebugImplementation & implementation()
{
    return ImplementationRegistry::current().debugImplementation();
}

void DebugMessage::hintImplementation(Implementation impl)
{
    ImplementationRegistry::current().initialize(impl);
}

DebugMessage::DebugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, const std::string & message)
: m_source(source)
, m_type(type)
, m_id(id)
, m_severity(severity)
, m_message(message)
{
}

GLenum DebugMessage::source() const
{
    return m_source;
}

GLenum DebugMessage::type() const
{
    return m_type;
}

GLuint DebugMessage::id() const
{
    return m_id;
}

GLenum DebugMessage::severity() const
{
    return m_severity;
}

const std::string & DebugMessage::message() const
{
    return m_message;
}

std::string DebugMessage::toString() const
{
    std::stringstream stream;

    stream
        << typeString()
        << ": " << std::hex << "0x" << m_id << std::dec
        << ", " << severityString() << " severity"
        << " (" << sourceString() << ")"
        << std::endl
        << "\t" << m_message;

    return stream.str();
}

std::string DebugMessage::severityString() const
{
    switch (m_severity)
    {
        case GL_DEBUG_SEVERITY_HIGH_ARB:
            return "high";
        case GL_DEBUG_SEVERITY_MEDIUM_ARB:
            return "medium";
        case GL_DEBUG_SEVERITY_LOW_ARB:
            return "low";
        default:
            return "unknown";
    }
}

std::string DebugMessage::sourceString() const
{
    switch (m_source)
    {
        case GL_DEBUG_SOURCE_API_ARB:
            return "API";
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:
            return "Window System";
        case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB:
            return "Shader Compiler";
        case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:
            return "Third Party";
        case GL_DEBUG_SOURCE_APPLICATION_ARB:
            return "Application";
        case GL_DEBUG_SOURCE_OTHER_ARB:
            return "Other";
        default:
            return "Unknown";
    }
}

std::string DebugMessage::typeString() const
{
    switch (m_type)
    {
        case GL_DEBUG_TYPE_OTHER_ARB:
            return "other";
        case GL_DEBUG_TYPE_ERROR_ARB:
            return "error";
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB:
            return "deprecated behavior";
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:
            return "undefined behavior";
        case GL_DEBUG_TYPE_PORTABILITY_ARB:
            return "portability";
        case GL_DEBUG_TYPE_PERFORMANCE_ARB:
            return "performance";
        default:
            return "unknown";
    }
}

bool DebugMessage::isFallbackImplementation()
{
    return implementation().isFallback();
}

void DebugMessage::enable(bool synchronous)
{
    implementation().enable();

    setSynchronous(synchronous);
}

void DebugMessage::disable()
{
    implementation().disable();
}

void DebugMessage::setCallback(Callback callback)
{
    implementation().setCallback(callback);
}

void DebugMessage::setSynchronous(bool synchronous)
{
    implementation().setSynchronous(synchronous);
}

void DebugMessage::insertMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char * message)
{
    assert(message != nullptr);

    insertMessage(DebugMessage(source, type, id, severity, std::string(message, length)));
}

void DebugMessage::insertMessage(GLenum source, GLenum type, GLuint id, GLenum severity, const std::string & message)
{
    insertMessage(DebugMessage(source, type, id, severity, message));
}

void DebugMessage::insertMessage(const DebugMessage & message)
{
    implementation().insertMessage(message);
}

void DebugMessage::enableMessage(GLenum source, GLenum type, GLenum severity, GLuint id)
{
    enableMessages(source, type, severity, 1, &id);
}

void DebugMessage::enableMessages(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint * ids)
{
    controlMessages(source, type, severity, count, ids, GL_TRUE);
}

void DebugMessage::enableMessages(GLenum source, GLenum type, GLenum severity, const std::vector<GLuint> & ids)
{
    enableMessages(source, type, severity, static_cast<int>(ids.size()), ids.data());
}

void DebugMessage::disableMessage(GLenum source, GLenum type, GLenum severity, GLuint id)
{
    disableMessages(source, type, severity, 1, &id);
}

void DebugMessage::disableMessages(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint * ids)
{
    controlMessages(source, type, severity, count, ids, GL_FALSE);
}

void DebugMessage::disableMessages(GLenum source, GLenum type, GLenum severity, const std::vector<GLuint> & ids)
{
    disableMessages(source, type, severity, static_cast<int>(ids.size()), ids.data());
}

void DebugMessage::controlMessages(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint * ids, GLboolean enabled)
{
    assert(ids != nullptr || count == 0);

    implementation().controlMessages(source, type, severity, count, ids, enabled);
}

} // namespace globjects
