#include <common/Context.h>

#include <cassert>

#include <glbinding/Binding.h>
#include <glbinding/Version.h>

#include <globjects/base/baselogging.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h> // specifies APIENTRY, should be after Error.h include,
                        // which requires APIENTRY in windows..


using namespace globjects;

Context::Context()
: m_swapInterval(VerticalSyncronization)
, m_window(nullptr)
{
}

Context::~Context()
{
    release();
}

GLFWwindow * Context::window()
{
    return m_window;
}

void Context::handleError(int errorCode, const char* errorMessage)
{
    globjects::fatal("GLFW error 0x%x;: %;", errorCode, errorMessage);
}

bool Context::create(const ContextFormat & format, const int width, const int height, GLFWmonitor * monitor)
{
    if (isValid())
    {
        globjects::warning() << "Context is already valid. Create was probably called before.";
        return true;
    }

    if (!glfwInit())
    {
        globjects::fatal() << "Could not initialize GLFW.";
        return false;
    }

    glfwSetErrorCallback(&Context::handleError);

    m_format = format;
    prepareFormat(m_format);

    
    m_window = glfwCreateWindow(width, height, "globjects", monitor, nullptr);

    if (!m_window)
    {
        globjects::fatal() << "Context creation failed (GLFW).";
        release();
        return false;
    }

    makeCurrent();
    glbinding::Binding::initialize(false);
    glfwSwapInterval(m_swapInterval);
    doneCurrent();

    // TODO: gather actual context format information and verify
    //ContextFormat::verify(format, m_format);

    return true;
}

void Context::prepareFormat(const ContextFormat & format)
{
    glbinding::Version version = validateVersion(format.version());

    if (!format.version().isNull() && format.version() != version)
    {
        globjects::warning() << "Changed unsupported OpenGL version from " << format.version().toString() << " to " << version.toString() << ".";
    }
    
    /*
     * GLFW3 does not set default hint values on window creation so at least
     * the default values must be set before glfwCreateWindow can be called.
     * cf. http://www.glfw.org/docs/latest/group__window.html#ga4fd9e504bb937e79588a0ffdca9f620b
     */
    glfwDefaultWindowHints();

#ifdef MAC_OS
  
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
#else
  
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, version.m_major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, version.m_minor);

    if (version >= glbinding::Version(3, 0))
    {
        if (format.forwardCompatible())
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
        if (format.debugContext())
            glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT,  true);
    }
  
    if (version >= glbinding::Version(3, 2))
    {
        glfwWindowHint(GLFW_OPENGL_PROFILE,
            format.profile() == ContextFormat::CoreProfile ? GLFW_OPENGL_CORE_PROFILE :
                (format.profile() == ContextFormat::CompatibilityProfile ? GLFW_OPENGL_COMPAT_PROFILE
                    : GLFW_OPENGL_ANY_PROFILE));
    }
  
#endif

    glfwWindowHint(GLFW_DEPTH_BITS, format.depthBufferSize());
    glfwWindowHint(GLFW_STENCIL_BITS, format.stencilBufferSize());
    glfwWindowHint(GLFW_RED_BITS, format.redBufferSize());
    glfwWindowHint(GLFW_GREEN_BITS, format.greenBufferSize());
    glfwWindowHint(GLFW_BLUE_BITS, format.blueBufferSize());
    glfwWindowHint(GLFW_ALPHA_BITS, format.alphaBufferSize());
    glfwWindowHint(GLFW_SAMPLES, format.samples());
}

void Context::release()
{
    if (!isValid())
        return;

    glfwDestroyWindow(m_window);
    m_window = nullptr;
}

void Context::swap()
{
    if (!isValid())
        return;

    glfwSwapBuffers(m_window);
}

bool Context::isValid() const
{
	return m_window != nullptr;
}

const ContextFormat & Context::format() const
{
	return m_format;
}

const std::string Context::swapIntervalString(const SwapInterval swapInterval)
{
	switch(swapInterval)
	{
        case NoVerticalSyncronization:
            return "NoVerticalSyncronization";
        case VerticalSyncronization:
            return "VerticalSyncronization";
        case AdaptiveVerticalSyncronization:
            return "AdaptiveVerticalSyncronization";
        default:
            return "";
	};
}

Context::SwapInterval Context::swapInterval() const
{
	return m_swapInterval;
}

void Context::setSwapInterval(const SwapInterval interval)
{
	if (interval == m_swapInterval)
		return;

    m_swapInterval = interval;

    makeCurrent();
    glfwSwapInterval(m_swapInterval);
    doneCurrent();
}

void Context::makeCurrent()
{
    if (!isValid())
        return;

    glfwMakeContextCurrent(m_window);
}

void Context::doneCurrent()
{
    if (!isValid())
        return;

    glfwMakeContextCurrent(0);
}

glbinding::Version Context::maximumSupportedVersion()
{
    glbinding::Version maxVersion;
    
    /*
     * GLFW3 does not set default hint values on window creation so at least
     * the default values must be set before glfwCreateWindow can be called.
     * cf. http://www.glfw.org/docs/latest/group__window.html#ga4fd9e504bb937e79588a0ffdca9f620b
     */
    glfwDefaultWindowHints();

#ifdef MAC_OS
    /*
     * Using OS X the following hints must be set for proper context initialization
     * (cf. http://stackoverflow.com/questions/19969937/getting-a-glsl-330-context-on-osx-10-9-mavericks)
     */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
#endif
    
    GLFWwindow * versionCheckWindow = glfwCreateWindow(1, 1, "VersionCheck", nullptr, nullptr);

    if (versionCheckWindow)
    {
        glfwMakeContextCurrent(versionCheckWindow);

        int majorVersion;
        int minorVersion;

        void (*getInteger)(unsigned int, int*) = reinterpret_cast<void (*)(unsigned int, int*)>(glfwGetProcAddress("glGetIntegerv"));

        if (getInteger != nullptr)
        {
            getInteger(0x821B, &majorVersion); // major version
            getInteger(0x821C, &minorVersion); // minor version

            maxVersion = glbinding::Version(majorVersion, minorVersion);
        }

        glfwDestroyWindow(versionCheckWindow);
   }

    return maxVersion;
}

glbinding::Version Context::validateVersion(const glbinding::Version & version)
{
    glbinding::Version maxVersion = maximumSupportedVersion();
    
    if (maxVersion.isNull())
    {
#ifdef MAC_OS
        maxVersion = glbinding::Version(3, 2);
#else
        maxVersion = glbinding::Version(3, 0);
#endif
      
    }

    if (version.isNull() || version > maxVersion)
    {
        return maxVersion;
    }

    if (!version.isValid())
    {
        glbinding::Version nearestValidVersion = version.nearest();
        if (nearestValidVersion > maxVersion)
        {
            return maxVersion;
        }

        return nearestValidVersion;
    }

    return version;
}
