
#include <glbinding/gl/gl.h>

#include <globjects/Buffer.h>
#include <globjects/Program.h>
#include <globjects/Shader.h>
#include <globjects/VertexArray.h>
#include <globjects/VertexAttributeBinding.h>
#include <globjects/DebugMessage.h>
#include <globjects/base/StaticStringSource.h>

#include <common/StringTemplate.h>
#include <common/Window.h>
#include <common/ContextFormat.h>
#include <common/Context.h>
#include <common/WindowEventHandler.h>
#include <common/events.h>

#include <ExampleWindowEventHandler.h>


using namespace gl;
using namespace glm;

namespace 
{
    const char * vertexShaderCode = R"(
#version 140
#extension GL_ARB_explicit_attrib_location : require

layout (location = 0) in vec2 corner;

out vec4 color;

void main()
{
    gl_Position = vec4(corner * 2.0 - 1.0, 0.0, 1.0);
    color = vec4(corner, 0.0, 1.0);
}

)";
    const char * fragmentShaderCode = R"(
#version 140
#extension GL_ARB_explicit_attrib_location : require

layout (location = 0) out vec4 fragColor;

in vec4 color;

void main()
{
    fragColor = color;
}

)";
}

class EventHandler : public ExampleWindowEventHandler
{
public:

    EventHandler()
    : m_vao(nullptr)
    , m_cornerBuffer(nullptr)
    , m_program(nullptr)
    {
    }

    virtual ~EventHandler()
    {
        m_vao->destroy();
        m_cornerBuffer->destroy();
        m_program->destroy();
    }

    virtual void initialize(Window & window) override
    {
        ExampleWindowEventHandler::initialize(window);

        globjects::DebugMessage::enable();

        glClearColor(0.2f, 0.3f, 0.4f, 1.f);

        m_cornerBuffer = new globjects::Buffer();
		m_program = new globjects::Program();
		m_vao = new globjects::VertexArray();

		m_program->attach(
            globjects::Shader::fromString(GL_VERTEX_SHADER,  vertexShaderCode),
            globjects::Shader::fromString(GL_FRAGMENT_SHADER, fragmentShaderCode));

        m_cornerBuffer->setData(std::array<vec2, 4>{ {
			vec2(0, 0), vec2(1, 0), vec2(0, 1), vec2(1, 1) } }, GL_STATIC_DRAW);

        m_vao->binding(0)->setAttribute(0);
		m_vao->binding(0)->setBuffer(m_cornerBuffer, 0, sizeof(vec2));
        m_vao->binding(0)->setFormat(2, GL_FLOAT);
        m_vao->enable(0);
    }
    
    virtual void framebufferResizeEvent(ResizeEvent & event) override
    {
        glViewport(0, 0, event.width(), event.height());
    }

    virtual void paintEvent(PaintEvent &) override
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_program->use();
        m_vao->drawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    virtual void idle(Window & window) override
    {
        window.repaint();
    }

private:
	globjects::VertexArray * m_vao;
	globjects::Buffer * m_cornerBuffer;
	globjects::Program * m_program;
};


int main(int /*argc*/, char * /*argv*/[])
{
    globjects::info() << "Usage:";
    globjects::info() << "\t" << "ESC" << "\t\t"       << "Close example";
    globjects::info() << "\t" << "ALT + Enter" << "\t" << "Toggle fullscreen";
    globjects::info() << "\t" << "F11" << "\t\t"       << "Toggle fullscreen";

    ContextFormat format;
    format.setVersion(3, 0);

    Window window;
    window.setEventHandler(new EventHandler());

    if (!window.create(format, "Wiki Example"))
        return 1;

    window.context()->setSwapInterval(Context::VerticalSyncronization);
    window.show();

    return MainLoop::run();
}
