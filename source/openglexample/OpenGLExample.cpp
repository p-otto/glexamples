#include "OpenGLExample.h"

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glbinding/gl/enum.h>
#include <glbinding/gl/bitfield.h>

#include <globjects/globjects.h>
#include <globjects/logging.h>
#include <globjects/DebugMessage.h>
#include <globjects/Program.h>

#include <widgetzeug/make_unique.hpp>

#include <gloperate/base/RenderTargetType.h>

#include <gloperate/painter/TargetFramebufferCapability.h>
#include <gloperate/painter/ViewportCapability.h>
#include <gloperate/painter/PerspectiveProjectionCapability.h>
#include <gloperate/painter/CameraCapability.h>
#include <gloperate/painter/VirtualTimeCapability.h>

#include <gloperate/primitives/AdaptiveGrid.h>
#include <gloperate/primitives/Icosahedron.h>

#include <globjects/VertexAttributeBinding.h>
#include <globjects/Program.h>
#include <globjects/Shader.h>
#include <globjects/Texture.h>
#include <globjects/AttachedTexture.h>

OpenGLExample::OpenGLExample(gloperate::ResourceManager & resourceManager, const std::string & relDataPath)
:   Painter("EmptyExample", resourceManager, relDataPath)
,   m_targetFramebufferCapability(addCapability(new gloperate::TargetFramebufferCapability()))
,   m_viewportCapability(addCapability(new gloperate::ViewportCapability()))
,   m_projectionCapability(addCapability(new gloperate::PerspectiveProjectionCapability(m_viewportCapability)))
,   m_cameraCapability(addCapability(new gloperate::CameraCapability()))
{
}

OpenGLExample::~OpenGLExample() = default;

void OpenGLExample::setupProjection()
{
    //static const auto zNear = 0.3f, zFar = 15.f, fovy = 50.f;
}

void OpenGLExample::onInitialize()
{
    // create program

    globjects::init();

#ifdef __APPLE__
    globjects::Shader::clearGlobalReplacements();
    globjects::Shader::globalReplace("#version 140", "#version 150");

    globjects::debug() << "Using global OS X shader replacement '#version 140' -> '#version 150'" << std::endl;
#endif

    m_texture = new globjects::Texture;
    // void image2D(gl::GLint level, gl::GLenum internalFormat, gl::GLsizei width, gl::GLsizei height, gl::GLint border, gl::GLenum format, gl::GLenum type, const gl::GLvoid * data);
    m_texture->image2D(0, gl::GL_RGBA, m_viewportCapability->width(), m_viewportCapability->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);

    m_fbo = new globjects::Framebuffer;
    m_fbo->attachTexture(gl::GL_COLOR_ATTACHMENT0, m_texture, 0);

    m_vertices = new globjects::Buffer;
    m_vertices->setData(std::vector<float>{
        -0.8f, -0.8f,
         0.8f, -0.8f,
         0.8f,  0.8f
    }, gl::GL_STATIC_DRAW);

    m_vao = new globjects::VertexArray;

    auto binding = m_vao->binding(0);
    binding->setAttribute(0);
    binding->setBuffer(m_vertices, 0, 2 * sizeof(float));
    binding->setFormat(2, gl::GL_FLOAT);

    m_vao->enable(0);

    m_program = new globjects::Program;
    m_program->attach(
        globjects::Shader::fromFile(gl::GL_VERTEX_SHADER, "data/openglexample/shader.vert"),
        globjects::Shader::fromFile(gl::GL_FRAGMENT_SHADER, "data/openglexample/shader.frag")
    );

    gl::glClearColor(1.0, 1.0, 1.0, 1.0);

    m_fbo->unbind();
}

void OpenGLExample::onPaint()
{
    if (m_viewportCapability->hasChanged())
    {
        m_texture->image2D(0, gl::GL_RGBA, m_viewportCapability->width(), m_viewportCapability->height(), 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr);

        m_viewportCapability->setChanged(false);
    }

    gl::glViewport(0, 0, m_viewportCapability->width() / 4, m_viewportCapability->height() / 4);

    m_vao->bind();
    m_program->use();

    m_fbo->bind();

    gl::glClear(gl::GL_COLOR_BUFFER_BIT);

    gl::glDrawArrays(gl::GL_TRIANGLES, 0, 3);

    std::array<int, 4> sourceRect = {{ 0, 0, m_viewportCapability->width() / 4, m_viewportCapability->height() / 4 }};
    std::array<int, 4> destRect = {{ 0, 0, m_viewportCapability->width(), m_viewportCapability->height() }};

    globjects::Framebuffer * targetFBO = m_targetFramebufferCapability->framebuffer() ? m_targetFramebufferCapability->framebuffer() : globjects::Framebuffer::defaultFBO();

    m_fbo->blit(gl::GL_COLOR_ATTACHMENT0, sourceRect, targetFBO, gl::GL_BACK_LEFT, destRect, gl::GL_COLOR_BUFFER_BIT, gl::GL_NEAREST);

    m_fbo->unbind();
}
