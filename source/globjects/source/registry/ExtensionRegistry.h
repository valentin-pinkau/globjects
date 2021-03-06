#pragma once

#include <set>
#include <string>

#include <glbinding/gl/extension.h>
#include <glbinding/Version.h>

#include <globjects/globjects_api.h>

namespace globjects
{

class Version;

class GLOBJECTS_API ExtensionRegistry
{
public:
    ExtensionRegistry();
    static ExtensionRegistry & current();

    std::set<gl::GLextension>::iterator begin();
    std::set<gl::GLextension>::iterator end();

    const std::set<gl::GLextension> & availableExtensions();
    const std::set<std::string> & unknownAvailableExtensions();

    bool hasExtension(gl::GLextension extension);
    bool hasExtension(const std::string & extensionName);
    bool isInCoreProfile(gl::GLextension extension);
    bool isInCoreProfile(gl::GLextension extension, const glbinding::Version & version);
protected:
    bool m_initialized;
    std::set<gl::GLextension> m_availableExtensions;
    std::set<std::string> m_unknownAvailableExtensions;

    void initialize();
};

} // namespace globjects
