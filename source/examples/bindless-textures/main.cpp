
#include <glm/gtc/random.hpp>
#include <glm/gtc/noise.hpp>

#include <glbinding/gl/gl.h>
#include <glbinding/gl/extension.h>

#include <globjects/globjects.h>
#include <globjects/logging.h>

#include <globjects/State.h>
#include <globjects/Capability.h>
#include <globjects/Texture.h>
#include <globjects/Program.h>
#include <globjects/Shader.h>
#include <globjects/VertexArray.h>
#include <globjects/VertexAttributeBinding.h>
#include <globjects/Buffer.h>
#include <globjects/DebugMessage.h>

#include <globjects/base/File.h>

#include <common/ScreenAlignedQuad.h>
#include <common/Camera.h>
#include <common/AxisAlignedBoundingBox.h>
#include <common/AbstractCoordinateProvider.h>
#include <common/WorldInHandNavigation.h>
#include <common/VertexDrawable.h>

#include <common/ContextFormat.h>
#include <common/Context.h>
#include <common/Window.h>
#include <common/WindowEventHandler.h>
#include <common/events.h>

#include <ExampleWindowEventHandler.h>


using namespace gl;
using namespace glm;

class EventHandler : public ExampleWindowEventHandler, AbstractCoordinateProvider
{
public:
    struct Vertex
    {
        vec3 position;
        vec2 texCoord;
        int side;
    };

    EventHandler()
    {
        m_aabb.extend(vec3(-8.f, -1.f, -8.f));
        m_aabb.extend(vec3( 8.f,  1.f,  8.f));

        m_camera = Camera(vec3( 0.f, 1.f, -1.f) * 2.f, vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f));

        m_nav.setCamera(&m_camera);
        m_nav.setCoordinateProvider(this);
        m_nav.setBoundaryHint(m_aabb);
    }

    virtual ~EventHandler()
    {
    }

    void createTextures()
    {
        const std::array<bvec3, 4> masks = { {
            bvec3( true, false, false),
            bvec3(false,  true, false),
            bvec3(false, false,  true),
            bvec3( true,  true, false) } };

        for (unsigned i = 0; i < m_textures.size(); ++i)
        {
            globjects::Texture * texture = globjects::Texture::createDefault(GL_TEXTURE_2D);

            static const int w(256);
            static const int h(256);

            using uchar = unsigned char;

            struct RGBA { uchar r; uchar g; uchar b; uchar a; };
            std::array<RGBA, w * h> data;

            for (unsigned j = 0; j < std::tuple_size<decltype(data)>::value; ++j)
            {
                int x = j % w;
                int y = j / w;

                vec2 pos(static_cast<float>(x) / w, static_cast<float>(y) / h);

                float h = perlin(pos * 16.f + vec2(3.f * i));

                vec3 color = vec3(masks[i % std::tuple_size<decltype(masks)>::value]) * h;
                ivec3 icolor = ivec3(color * 255.f);

                data[j] = RGBA{ static_cast<uchar>(icolor.r), static_cast<uchar>(icolor.g), static_cast<uchar>(icolor.b), 255 };
            }

            texture->image2D(0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
            m_textures[i] = texture;
        }
    }

    void createGeometry()
    {
        std::array<vec3, 8> points = { {
            vec3(sin(radians(0.f)), 0.f, cos(radians(0.f))),
            vec3(sin(radians(120.f)), 0.f, cos(radians(120.f))),
            vec3(sin(radians(240.f)), 0.f, cos(radians(240.f))),
            vec3(0.0, glm::sqrt(pow(2.f / cos(radians(30.f)) - 1.f, 2.f)), 0.f) } };

        std::array<Vertex, 6> vertices = { {
            Vertex{ points[0], vec2(0.0f, 0.0f), 0 },
            Vertex{ points[1], vec2(1.0f, 0.0f), 0 },
            Vertex{ points[3], vec2(0.5f, 1.0f), 0 },
            Vertex{ points[2], vec2(0.0f, 0.0f), 1 },
            Vertex{ points[0], vec2(1.0f, 0.0f), 2 },
            Vertex{ points[1], vec2(0.5f, 1.0f), 3 } } };

        m_drawable = new VertexDrawable(vertices, GL_TRIANGLE_STRIP);

        m_drawable->setFormats({
            Format(3, GL_FLOAT, offsetof(Vertex, position)),
            Format(2, GL_FLOAT, offsetof(Vertex, texCoord)),
            FormatI(1, GL_INT, offsetof(Vertex, side)) });

        m_drawable->enableAll();
    }

    virtual void initialize(Window & window) override
    {
        ExampleWindowEventHandler::initialize(window);

        globjects::DebugMessage::enable();

        if (!globjects::hasExtension(GLextension::GL_NV_bindless_texture))
        {
            globjects::critical() << "Blindess textures are not supported";

            window.close();
            return;
        }

        globjects::ref_ptr<globjects::State> state = new globjects::State;
        state->enable(GL_CULL_FACE);
        state->clearColor(0.2f, 0.3f, 0.4f, 1.f);

        createGeometry();
        createTextures();

        m_program = new globjects::Program;
        m_program->attach(
            globjects::Shader::fromFile(GL_VERTEX_SHADER,   "data/bindless-textures/shader.vert"),
            globjects::Shader::fromFile(GL_FRAGMENT_SHADER, "data/bindless-textures/shader.frag"));

        std::array<globjects::TextureHandle, std::tuple_size<decltype(m_textures)>::value> handles;
        for (unsigned i = 0; i < m_textures.size(); ++i)
            handles[i] = m_textures[i]->makeResident();

        m_program->setUniform("textures", handles);

        window.addTimer(0, 0);
    }
    
    virtual void framebufferResizeEvent(ResizeEvent & event) override
    {
        int width = event.width();
        int height = event.height();

        glViewport(0, 0, width, height);

        m_camera.setViewport(width, height);
    }

    virtual void paintEvent(PaintEvent &) override
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_program->setUniform("projection", m_camera.viewProjection());

        m_program->use();
        m_drawable->draw();
        m_program->release();
    }

    virtual void timerEvent(TimerEvent & event) override
    {        
        event.window()->repaint();
    }

    virtual void keyPressEvent(KeyEvent & event) override
    {
        switch (event.key())
        {
        case GLFW_KEY_F5:
            globjects::File::reloadAll();
            break;

        case GLFW_KEY_SPACE:
            m_nav.reset();
            event.window()->repaint();
            break;
        }
    }

    virtual void mousePressEvent(MouseEvent & event) override
    {
        switch (event.button())
        {
        case GLFW_MOUSE_BUTTON_LEFT:
            m_nav.panBegin(event.pos());
            event.accept();
            break;

        case GLFW_MOUSE_BUTTON_RIGHT:
            m_nav.rotateBegin(event.pos());
            event.accept();
            break;
        }
    }
    virtual void mouseMoveEvent(MouseEvent & event) override
    {
        switch (m_nav.mode())
        {
        case WorldInHandNavigation::PanInteraction:
            m_nav.panProcess(event.pos());
            event.accept();
            break;

        case WorldInHandNavigation::RotateInteraction:
            m_nav.rotateProcess(event.pos());
            event.accept();

        default:
            break;
        }
    }
    virtual void mouseReleaseEvent(MouseEvent & event) override
    {
        switch (event.button())
        {
        case GLFW_MOUSE_BUTTON_LEFT:
            m_nav.panEnd();
            event.accept();
            break;

        case GLFW_MOUSE_BUTTON_RIGHT:
            m_nav.rotateEnd();
            event.accept();
            break;
        }
    }

    virtual void scrollEvent(ScrollEvent & event) override
    {
        if (WorldInHandNavigation::NoInteraction != m_nav.mode())
            return;

        m_nav.scaleAtMouse(event.pos(), -event.offset().y * 0.1f);
        event.accept();
    }

    virtual float depthAt(const ivec2 & windowCoordinates) const override
    {
        return AbstractCoordinateProvider::depthAt(m_camera, GL_DEPTH_COMPONENT, windowCoordinates);
    }

    virtual vec3 objAt(const ivec2 & windowCoordinates) const override
    {
        return unproject(m_camera, static_cast<GLenum>(GL_DEPTH_COMPONENT), windowCoordinates);
    }

    virtual vec3 objAt(const ivec2 & windowCoordinates, const float depth) const override
    {
        return unproject(m_camera, depth, windowCoordinates);
    }

    virtual vec3 objAt(
        const ivec2 & windowCoordinates
    ,   const float depth
    ,   const mat4 & viewProjectionInverted) const override
    {
        return unproject(m_camera, viewProjectionInverted, depth, windowCoordinates);
    }

protected:
    Camera m_camera;

    WorldInHandNavigation m_nav;
    AxisAlignedBoundingBox m_aabb;

    std::array<globjects::ref_ptr<globjects::Texture>, 4> m_textures;
    globjects::ref_ptr<globjects::Program> m_program;
    globjects::ref_ptr<VertexDrawable> m_drawable;
};


int main(int /*argc*/, char * /*argv*/[])
{
    globjects::info() << "Usage:";
    globjects::info() << "\t" << "ESC" << "\t\t"        << "Close example";
    globjects::info() << "\t" << "ALT + Enter" << "\t"  << "Toggle fullscreen";
    globjects::info() << "\t" << "F11" << "\t\t"        << "Toggle fullscreen";
    globjects::info() << "\t" << "F5" << "\t\t"         << "Reload shaders";
    globjects::info() << "\t" << "Space" << "\t\t"      << "Reset camera";
    globjects::info() << "\t" << "Left Mouse" << "\t"   << "Pan scene";
    globjects::info() << "\t" << "Right Mouse" << "\t"  << "Rotate scene";
    globjects::info() << "\t" << "Mouse Wheel" << "\t"  << "Zoom scene";

    ContextFormat format;
    format.setVersion(3, 0);

    Window window;
    window.setEventHandler(new EventHandler());

    if (!window.create(format, "Bindless Texture Example"))
        return 1;

    window.context()->setSwapInterval(Context::NoVerticalSyncronization);
    window.show();

    return MainLoop::run();
}
