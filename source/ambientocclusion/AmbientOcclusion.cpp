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

void AmbientOcclusion::setupFramebuffers()
{
    if (m_multisampling)
    {
        m_colorAttachment = new Texture(GL_TEXTURE_2D_MULTISAMPLE);
        m_colorAttachment->bind(); // workaround
        m_normalAttachment = new Texture(GL_TEXTURE_2D_MULTISAMPLE);
        m_normalAttachment->bind();
        m_depthAttachment = new Texture(GL_TEXTURE_2D_MULTISAMPLE);
        m_depthAttachment->bind();
        
        m_occlusionAttachment = new Texture(GL_TEXTURE_2D_MULTISAMPLE);
        m_occlusionAttachment->bind();
        
        m_blurAttachment = new Texture(GL_TEXTURE_2D_MULTISAMPLE);
        m_blurAttachment->bind();
    }
    else
    {
        m_colorAttachment = Texture::createDefault(GL_TEXTURE_2D);
        m_normalAttachment = Texture::createDefault(GL_TEXTURE_2D);
        m_depthAttachment = Texture::createDefault(GL_TEXTURE_2D);
        
        m_occlusionAttachment = Texture::createDefault(GL_TEXTURE_2D);
        
        m_blurAttachment = Texture::createDefault(GL_TEXTURE_2D);
    }
    
    m_modelFbo = make_ref<Framebuffer>();
    m_modelFbo->attachTexture(GL_COLOR_ATTACHMENT0, m_colorAttachment);
    m_modelFbo->attachTexture(GL_COLOR_ATTACHMENT1, m_normalAttachment);
    m_modelFbo->attachTexture(GL_DEPTH_ATTACHMENT, m_depthAttachment);
    m_modelFbo->setDrawBuffers({GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1});
    
    m_occlusionFbo = make_ref<Framebuffer>();
    m_occlusionFbo->attachTexture(GL_COLOR_ATTACHMENT0, m_occlusionAttachment);
    
    m_blurFbo = make_ref<Framebuffer>();
    m_blurFbo->attachTexture(GL_COLOR_ATTACHMENT0, m_blurAttachment);
    
    updateFramebuffers();
    
    m_modelFbo->printStatus(true);
    m_occlusionFbo->printStatus(true);
}

void AmbientOcclusion::setupModel()
{
    const auto meshLoader = gloperate_assimp::AssimpMeshLoader{};
    const auto scene = meshLoader.load("data/ambientocclusion/teapot.obj", nullptr);
    m_model = gloperate::make_unique<gloperate::PolygonalDrawable>(*scene);
    
    m_grid = make_ref<gloperate::AdaptiveGrid>();
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
                            Shader::fromFile(GL_VERTEX_SHADER, "data/ambientocclusion/screen_quad.vert"),
                            Shader::fromFile(GL_FRAGMENT_SHADER, "data/ambientocclusion/ssao_crytek.frag")
    );
    
    m_blurProgram = new Program{};
    m_blurProgram->attach(
                            Shader::fromFile(GL_VERTEX_SHADER, "data/ambientocclusion/screen_quad.vert"),
                            Shader::fromFile(GL_FRAGMENT_SHADER, "data/ambientocclusion/blur.frag")
    );
    
    m_mixProgram = new Program{};
    m_mixProgram->attach(
                            Shader::fromFile(GL_VERTEX_SHADER, "data/ambientocclusion/screen_quad.vert"),
                            Shader::fromFile(GL_FRAGMENT_SHADER, "data/ambientocclusion/mix.frag")
    );
}

void AmbientOcclusion::updateFramebuffers()
{
    static const auto numSamples = 4u;
    const auto width = m_viewportCapability->width(), height = m_viewportCapability->height();
    
    if (m_multisampling)
    {
        m_colorAttachment->image2DMultisample(numSamples, GL_RGBA8, width, height, GL_TRUE);
        m_normalAttachment->image2DMultisample(numSamples, GL_RGB8, width, height, GL_TRUE);
        m_depthAttachment->image2DMultisample(numSamples, GL_DEPTH_COMPONENT, width, height, GL_TRUE);
        
        m_occlusionAttachment->image2DMultisample(numSamples, GL_R8, width, height, GL_TRUE);
        
        m_blurAttachment->image2DMultisample(numSamples, GL_R8, width, height, GL_TRUE);
    }
    else
    {
        m_colorAttachment->image2D(0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        m_normalAttachment->image2D(0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        m_depthAttachment->image2D(0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);
        
        m_occlusionAttachment->image2D(0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
        
        m_blurAttachment->image2D(0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
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
    
    setupFramebuffers();
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

    m_modelFbo->bind(GL_FRAMEBUFFER);
    m_modelFbo->clearBuffer(GL_COLOR, 0, glm::vec4{0.85f, 0.87f, 0.91f, 1.0f});
    m_modelFbo->clearBuffer(GL_COLOR, 1, glm::vec4{0.0f, 1.0f, 0.0f, 0.0f});
    m_modelFbo->clearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0.0f);
    
    glEnable(GL_DEPTH_TEST);
    
    const auto transform = m_projectionCapability->projection() * m_cameraCapability->view();
    const auto eye = m_cameraCapability->eye();
    
    m_grid->update(eye, transform);
    m_grid->draw();

    m_modelProgram->use();
    m_modelProgram->setUniform(m_transformLocation, transform);
    
    m_model->draw();
    
    m_modelProgram->release();
    
    // calculate ambient occlusion
    m_screenAlignedQuad = new gloperate::ScreenAlignedQuad(m_ambientOcclusionProgram);
    m_screenAlignedQuad->program()->use();
    
    m_occlusionFbo->bind();
    m_occlusionFbo->clearBuffer(GL_COLOR, 0, glm::vec4{0.0, 0.0, 0.0, 0.0});
    
    m_depthAttachment->bindActive(GL_TEXTURE0);
    m_screenAlignedQuad->program()->setUniform("u_depth", 0);
    m_normalAttachment->bindActive(GL_TEXTURE1);
    m_screenAlignedQuad->program()->setUniform("u_normal", 1);
    
    m_screenAlignedQuad->draw();
    
    // blur ambient occlusion texture
    m_screenAlignedQuad = new gloperate::ScreenAlignedQuad(m_blurProgram);
    m_screenAlignedQuad->program()->use();
    
    m_blurFbo->bind();
    m_blurFbo->clearBuffer(GL_COLOR, 0, glm::vec4{0.0, 0.0, 0.0, 0.0});
    
    m_occlusionAttachment->bindActive(GL_TEXTURE0);
    m_screenAlignedQuad->program()->setUniform("u_occlusion", 0);
    
    m_screenAlignedQuad->draw();
    
    // finally, render to screen
    auto default_framebuffer = m_targetFramebufferCapability->framebuffer();
    
    auto draw_buffer = GL_COLOR_ATTACHMENT0;
    if (!default_framebuffer) {
        default_framebuffer = globjects::Framebuffer::defaultFBO();
        draw_buffer = GL_BACK_LEFT;
    }
    
    default_framebuffer->bind(GL_FRAMEBUFFER);
    default_framebuffer->clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    m_screenAlignedQuad = new gloperate::ScreenAlignedQuad(m_mixProgram);
    m_screenAlignedQuad->program()->use();
    
    m_colorAttachment->bindActive(GL_TEXTURE0);
    m_screenAlignedQuad->program()->setUniform("u_color", 0);
    m_blurAttachment->bindActive(GL_TEXTURE1);
    m_screenAlignedQuad->program()->setUniform("u_blur", 1);
    
    m_screenAlignedQuad->draw();
}
