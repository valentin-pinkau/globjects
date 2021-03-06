#include <common/ExampleWindowEventHandler.h>

#include <globjects/globjects.h>

#include <common/events.h>
#include <common/Window.h>

#include <sstream>
#include <iomanip>


using namespace gl;

ExampleWindowEventHandler::ExampleWindowEventHandler()
:   m_swapElapsedTime(0.0)
,   m_swapCount(0)
{
}

ExampleWindowEventHandler::~ExampleWindowEventHandler()
{
}

void ExampleWindowEventHandler::initialize(Window & /*window*/)
{
    globjects::init();
}

void ExampleWindowEventHandler::handleEvent(WindowEvent & event)
{
    WindowEventHandler::handleEvent(event);

    if (!event.window())
        return;

    switch (event.type())
    {
    case WindowEvent::FrameBufferResize:
        setViewport(static_cast<ResizeEvent&>(event));
        break;

    case WindowEvent::Paint:
        computeFps(static_cast<PaintEvent&>(event));
        break;

    case WindowEvent::KeyPress:
        handleDefaultKeys(static_cast<KeyEvent&>(event));
        break;

    default:
        break;
    }
}

void ExampleWindowEventHandler::setViewport(ResizeEvent & event)
{
    gl::glViewport(0, 0, event.width(), event.height());
}

namespace 
{

    bool startsWith(const std::string & str, const std::string str2)
    {
        return str.compare(0, str2.length(), str2) == 0;
    }

}

void ExampleWindowEventHandler::computeFps(PaintEvent & event)
{
    m_timer.update();

    ++m_swapCount;

    if (m_timer.elapsed().count() - m_swapElapsedTime >= 1e+9)
    {
        const float fps = 1e+9f * static_cast<float>(static_cast<long double>(m_swapCount) / (m_timer.elapsed().count() - m_swapElapsedTime));

        std::string title = event.window()->title();
        if (!startsWith(title, m_baseTitle) || m_baseTitle.length() == 0)
            m_baseTitle = title;

        std::stringstream stream;
        stream << m_baseTitle << " (" << std::fixed << std::setprecision(2) << fps << " fps)";

        event.window()->setTitle(stream.str());

        m_swapElapsedTime = static_cast<long double>(m_timer.elapsed().count());
        m_swapCount = 0;
    }
}

void ExampleWindowEventHandler::handleDefaultKeys(KeyEvent & event)
{
    switch (event.key())
    {
    case GLFW_KEY_ESCAPE:
        event.window()->close();
        break;
    case GLFW_KEY_ENTER:
        if ((event.modifiers() & GLFW_MOD_ALT) == 0)
        {
            break;
        }
        // fall through
    case GLFW_KEY_F11:
        event.window()->toggleMode();
        break;
    default:
        break;
    }
}
