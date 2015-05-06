#include "AmbientOcclusion.h"

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glbinding/gl/enum.h>
#include <glbinding/gl/bitfield.h>
#include <glbinding/gl/boolean.h>

#include <globjects/globjects.h>
#include <globjects/logging.h>
#include <globjects/DebugMessage.h>
#include <globjects/Program.h>
#include <globjects/Texture.h>

#include <gloperate/base/RenderTargetType.h>

#include <gloperate/painter/TargetFramebufferCapability.h>
#include <gloperate/painter/ViewportCapability.h>
#include <gloperate/painter/PerspectiveProjectionCapability.h>
#include <gloperate/painter/CameraCapability.h>
#include <gloperate/painter/VirtualTimeCapability.h>

#include <gloperate/primitives/AdaptiveGrid.h>
#include <gloperate/base/make_unique.hpp>

using namespace gl;
using namespace glm;
using namespace globjects;

AmbientOcclusion::AmbientOcclusion(gloperate::ResourceManager & resourceManager)
:   Painter(resourceManager)
,   m_targetFramebufferCapability(addCapability(new gloperate::TargetFramebufferCapability()))
,   m_viewportCapability(addCapability(new gloperate::ViewportCapability()))
,   m_projectionCapability(addCapability(new gloperate::PerspectiveProjectionCapability(m_viewportCapability)))
,   m_cameraCapability(addCapability(new gloperate::CameraCapability()))
{
}

AmbientOcclusion::~AmbientOcclusion() = default;

void AmbientOcclusion::setupProjection()
{
    static const auto zNear = 0.3f, zFar = 15.f, fovy = 50.f;

    m_projectionCapability->setZNear(zNear);
    m_projectionCapability->setZFar(zFar);
    m_projectionCapability->setFovy(radians(fovy));

    m_grid->setNearFar(zNear, zFar);
}

void AmbientOcclusion::setupFramebuffer()
{
    if (m_multisampling)
    {
        m_colorAttachment = new Texture(GL_TEXTURE_2D_MULTISAMPLE);
        m_colorAttachment->bind(); // workaround
        m_normalAttachment = new Texture(GL_TEXTURE_2D_MULTISAMPLE);
        m_normalAttachment->bind();
        m_depthAttachment = new Texture(GL_TEXTURE_2D_MULTISAMPLE);
        m_depthAttachment->bind();
    }
    else
    {
        m_colorAttachment = Texture::createDefault(GL_TEXTURE_2D);
        m_normalAttachment = Texture::createDefault(GL_TEXTURE_2D);
        m_depthAttachment = Texture::createDefault(GL_TEXTURE_2D);
    }
    
    m_fbo = make_ref<Framebuffer>();
    
    m_fbo->attachTexture(GL_COLOR_ATTACHMENT0, m_colorAttachment);
    m_fbo->attachTexture(GL_COLOR_ATTACHMENT1, m_normalAttachment);
    m_fbo->attachTexture(GL_DEPTH_ATTACHMENT, m_depthAttachment);
    
    updateFramebuffer();
    
    m_fbo->printStatus(true);
}

void AmbientOcclusion::setupModel()
{
    const auto meshLoader = gloperate_assimp::AssimpMeshLoader{};
    const auto scene = meshLoader.load("data/ambientocclusion/teapot.obj", nullptr);
    m_model = gloperate::make_unique<gloperate::PolygonalDrawable>(*scene);
    
    m_grid = new gloperate::AdaptiveGrid{};
    m_grid->setColor({0.6f, 0.6f, 0.6f});
}

void AmbientOcclusion::setupShaders()
{
    m_modelProgram = new Program{};
    m_modelProgram->attach(
                           Shader::fromFile(GL_VERTEX_SHADER, "data/ambientocclusion/model.vert"),
                           Shader::fromFile(GL_FRAGMENT_SHADER, "data/ambientocclusion/model.frag")
    );
    
    m_transformLocation = m_modelProgram->getUniformLocation("transform");
    
    m_ambientOcclusionProgram = new Program{};
    m_ambientOcclusionProgram->attach(
                           Shader::fromFile(GL_VERTEX_SHADER, "data/ambientocclusion/ssao_crytek.vert"),
                           Shader::fromFile(GL_FRAGMENT_SHADER, "data/ambientocclusion/ssao_crytek.frag")
    );
    
    m_blurProgram = new Program{};
    m_blurProgram->attach(
                          Shader::fromFile(GL_VERTEX_SHADER, "data/ambientocclusion/blur.vert"),
                          Shader::fromFile(GL_FRAGMENT_SHADER, "data/ambientocclusion/blur.frag")
    );
}

void AmbientOcclusion::updateFramebuffer()
{
    static const auto numSamples = 4u;
    const auto width = m_viewportCapability->width(), height = m_viewportCapability->height();
    
    if (m_multisampling)
    {
        m_colorAttachment->image2DMultisample(numSamples, GL_RGBA8, width, height, GL_TRUE);
        m_normalAttachment->image2DMultisample(numSamples, GL_RGB8, width, height, GL_TRUE);
        m_depthAttachment->image2DMultisample(numSamples, GL_DEPTH_COMPONENT, width, height, GL_TRUE);
    }
    else
    {
        m_colorAttachment->image2D(0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        m_normalAttachment->image2D(0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        m_depthAttachment->image2D(0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);
    }
}

void AmbientOcclusion::onInitialize()
{
    // create program
    globjects::init();
    
#ifdef __APPLE__
    Shader::clearGlobalReplacements();
    Shader::globalReplace("#version 140", "#version 150");

    debug() << "Using global OS X shader replacement '#version 140' -> '#version 150'" << std::endl;
#endif

    glClearColor(0.85f, 0.87f, 0.91f, 1.0f);

    // some magic numbers that give a good view on the teapot
    m_cameraCapability->setEye(glm::vec3(0.0f, 1.7f, -2.0f));
    m_cameraCapability->setCenter(glm::vec3(0.2f, 0.3f, 0.0f));
    
    setupFramebuffer();
    setupModel();
    setupShaders();
    setupProjection();
}

void AmbientOcclusion::onPaint()
{
    if (m_viewportCapability->hasChanged())
    {
        glViewport(
            m_viewportCapability->x(),
            m_viewportCapability->y(),
            m_viewportCapability->width(),
            m_viewportCapability->height());

        m_viewportCapability->setChanged(false);
    }

    m_fbo->bind(GL_FRAMEBUFFER);
    m_fbo->clearBuffer(GL_COLOR, 0, glm::vec4{0.85f, 0.87f, 0.91f, 1.0f});
    m_fbo->clearBuffer(GL_COLOR, 1, glm::vec4{0.0f, 0.0f, 0.0f, 0.0f});
    m_fbo->clearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0.0f);
    
    glEnable(GL_DEPTH_TEST);
    
    const auto transform = m_projectionCapability->projection() * m_cameraCapability->view();
    const auto eye = m_cameraCapability->eye();
    
    m_grid->update(eye, transform);
    m_grid->draw();

    m_modelProgram->use();
    m_modelProgram->setUniform(m_transformLocation, transform);
    
    m_model->draw();
    
    m_modelProgram->release();
    
    Framebuffer::unbind(GL_FRAMEBUFFER);
    
    const auto rect = std::array<gl::GLint, 4>{{
        m_viewportCapability->x(),
        m_viewportCapability->y(),
        m_viewportCapability->width(),
        m_viewportCapability->height()}};
    
    auto targetfbo = m_targetFramebufferCapability->framebuffer();
    auto drawBuffer = GL_COLOR_ATTACHMENT0;
    
    if (!targetfbo)
    {
        targetfbo = globjects::Framebuffer::defaultFBO();
        drawBuffer = GL_BACK_LEFT;
    }
    
    m_fbo->blit(GL_COLOR_ATTACHMENT0, rect, targetfbo, drawBuffer, rect,
                GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}
