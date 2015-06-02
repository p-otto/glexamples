#include "AmbientOcclusion.h"

#include "Plane.h"
#include "ScreenAlignedQuadRenderer.h"
#include "AmbientOcclusionOptions.h"
#include "AmbientOcclusionStage.h"

#include <chrono>

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glbinding/gl/enum.h>
#include <glbinding/gl/bitfield.h>
#include <glbinding/gl/boolean.h>

#include <globjects/globjects.h>
#include <globjects/logging.h>
#include <globjects/DebugMessage.h>
#include <globjects/Program.h>
#include <globjects/Texture.h>
#include <globjects/Framebuffer.h>

#include <gloperate/base/RenderTargetType.h>

#include <gloperate/painter/TargetFramebufferCapability.h>
#include <gloperate/painter/ViewportCapability.h>
#include <gloperate/painter/PerspectiveProjectionCapability.h>
#include <gloperate/painter/CameraCapability.h>
#include <gloperate/painter/VirtualTimeCapability.h>

#include <gloperate/primitives/Scene.h>
#include <gloperate/primitives/PolygonalDrawable.h>
#include <gloperate/primitives/AdaptiveGrid.h>

#include <gloperate-assimp/AssimpMeshLoader.h>

#include <gloperate/base/make_unique.hpp>

#include <gloperate/resources/ResourceManager.h>

using namespace gl;
using namespace glm;
using namespace globjects;

AmbientOcclusion::AmbientOcclusion(gloperate::ResourceManager & resourceManager)
:   Painter(resourceManager)
,   m_targetFramebufferCapability(addCapability(new gloperate::TargetFramebufferCapability()))
,   m_viewportCapability(addCapability(new gloperate::ViewportCapability()))
,   m_projectionCapability(addCapability(new gloperate::PerspectiveProjectionCapability(m_viewportCapability)))
,   m_cameraCapability(addCapability(new gloperate::CameraCapability()))
,   m_occlusionOptions(new AmbientOcclusionOptions(*this))
,   m_ambientOcclusionStage(gloperate::make_unique<AmbientOcclusionStage>(m_occlusionOptions))
{
}

AmbientOcclusion::~AmbientOcclusion() = default;

void AmbientOcclusion::setupProjection()
{
    static const auto zNear = 0.3f, zFar = 150.f, fovy = 50.f;

    m_projectionCapability->setZNear(zNear);
    m_projectionCapability->setZFar(zFar);
    m_projectionCapability->setFovy(radians(fovy));

    m_grid->setNearFar(zNear, zFar);
}

void AmbientOcclusion::setupFramebuffers()
{
    m_colorAttachment = Texture::createDefault(GL_TEXTURE_2D);
    m_normalDepthAttachment = Texture::createDefault(GL_TEXTURE_2D);
    m_depthBuffer = Texture::createDefault(GL_TEXTURE_2D);
    
    m_blurAttachment = Texture::createDefault(GL_TEXTURE_2D);
    m_blurTmpAttachment = Texture::createDefault(GL_TEXTURE_2D);
    
    m_modelFbo = make_ref<Framebuffer>();
    m_modelFbo->attachTexture(GL_COLOR_ATTACHMENT0, m_colorAttachment);
    m_modelFbo->attachTexture(GL_COLOR_ATTACHMENT1, m_normalDepthAttachment);
    m_modelFbo->attachTexture(GL_DEPTH_ATTACHMENT, m_depthBuffer);
    m_modelFbo->setDrawBuffers({GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1});
    
    m_blurFbo = make_ref<Framebuffer>();
    m_blurFbo->attachTexture(GL_COLOR_ATTACHMENT0, m_blurAttachment);
    m_blurTmpFbo = make_ref<Framebuffer>();
    m_blurTmpFbo->attachTexture(GL_COLOR_ATTACHMENT0, m_blurTmpAttachment);
    
    updateFramebuffers();
    
    m_modelFbo->printStatus(true);
    m_blurFbo->printStatus(true);
    m_blurTmpFbo->printStatus(true);
}

void AmbientOcclusion::setupModel()
{
    auto scene = m_resourceManager.load<gloperate::Scene>("data/ambientocclusion/dragon.obj");
    for (auto & mesh : scene->meshes())
    {
        m_drawables.push_back(gloperate::PolygonalDrawable(*mesh));
    }

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
    
    m_phongProgram = new Program{};
    m_phongProgram->attach(
                           Shader::fromFile(GL_VERTEX_SHADER, "data/ambientocclusion/model.vert"),
                           Shader::fromFile(GL_FRAGMENT_SHADER, "data/ambientocclusion/phong.frag")
    );

    m_blurXProgram = new Program{};
    m_blurXProgram->attach(
                            Shader::fromFile(GL_VERTEX_SHADER, "data/ambientocclusion/screen_quad.vert"),
                            Shader::fromFile(GL_FRAGMENT_SHADER, "data/ambientocclusion/blur_x.frag")
    );
    
    m_blurYProgram = new Program{};
    m_blurYProgram->attach(
                            Shader::fromFile(GL_VERTEX_SHADER, "data/ambientocclusion/screen_quad.vert"),
                            Shader::fromFile(GL_FRAGMENT_SHADER, "data/ambientocclusion/blur_y.frag")
    );
    
    m_mixProgram = new Program{};
    m_mixProgram->attach(
                            Shader::fromFile(GL_VERTEX_SHADER, "data/ambientocclusion/screen_quad.vert"),
                            Shader::fromFile(GL_FRAGMENT_SHADER, "data/ambientocclusion/mix.frag")
    );
}

void AmbientOcclusion::updateFramebuffers()
{
    const auto width = m_viewportCapability->width(), height = m_viewportCapability->height();
    
    m_colorAttachment->image2D(0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    m_normalDepthAttachment->image2D(0, GL_RGBA16, width, height, 0, GL_RGBA, GL_UNSIGNED_SHORT, nullptr);
    m_depthBuffer->image2D(0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);

    m_ambientOcclusionStage->updateFramebuffer(width, height);

    m_blurAttachment->image2D(0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    m_blurTmpAttachment->image2D(0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
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

    m_screenAlignedQuad = gloperate::make_unique<ScreenAlignedQuadRenderer>();
    m_plane = gloperate::make_unique<Plane>();
    
    // some magic numbers that give a good view on the teapot
    m_cameraCapability->setEye(glm::vec3(0.0f, 15.7f, -15.0f));
    m_cameraCapability->setCenter(glm::vec3(0.2f, 0.3f, 0.0f));
    
    setupFramebuffers();
    setupModel();
    setupShaders();
    setupProjection();
    m_ambientOcclusionStage->setupKernelAndRotationTex();
}

void AmbientOcclusion::drawScene()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    
    auto program = m_occlusionOptions->phong() ? m_phongProgram : m_modelProgram;

    program->use();
    
    glm::mat4 model;
    program->setUniform("u_mvp", m_projectionCapability->projection() * m_cameraCapability->view() * model);
    program->setUniform("u_modelView", m_cameraCapability->view() * model);
    program->setUniform("u_farPlane", m_projectionCapability->zFar());

    m_plane->draw();
    for (auto & drawable : m_drawables)
    {
        drawable.draw();
    }

    program->release();

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
}


void AmbientOcclusion::onPaint()
{
    if (m_viewportCapability->hasChanged() || m_occlusionOptions->hasResolutionChanged()) {
        glViewport(
            m_viewportCapability->x(),
            m_viewportCapability->y(),
            m_viewportCapability->width(),
            m_viewportCapability->height());

        m_viewportCapability->setChanged(false);
        updateFramebuffers();
    }
    
    switch (m_occlusionOptions->ambientOcclusion()) {
        case None:
            drawWithoutAmbientOcclusion();
            break;
        case ScreenSpace:
            drawScreenSpaceAmbientOcclusion();
            break;
    }

    drawGrid();
}

void AmbientOcclusion::drawGrid()
{
    // move grid below plane
    glm::mat4 model;
    model = glm::translate(model, glm::vec3(0.0f, -0.1f, 0.0f));
    const auto transform = m_projectionCapability->projection() * m_cameraCapability->view() * model;
    const auto eye = m_cameraCapability->eye();
    
    glEnable(GL_DEPTH_TEST);
    
    m_grid->update(eye, transform);
    m_grid->draw();

    glDisable(GL_DEPTH_TEST);
}

void AmbientOcclusion::drawWithoutAmbientOcclusion() {
    auto default_framebuffer = m_targetFramebufferCapability->framebuffer();
    if (!default_framebuffer) {
        default_framebuffer = globjects::Framebuffer::defaultFBO();
    }

    default_framebuffer->bind(GL_FRAMEBUFFER);
    default_framebuffer->clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawScene();
}

void AmbientOcclusion::drawScreenSpaceAmbientOcclusion()
{
    m_modelFbo->bind(GL_FRAMEBUFFER);
    m_modelFbo->clearBuffer(GL_COLOR, 0, glm::vec4{0.85f, 0.87f, 0.91f, 1.0f});
    m_modelFbo->clearBuffer(GL_COLOR, 1, glm::vec4{0.0f, 0.0f, 0.0f, 1.0f});
    m_modelFbo->clearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0);

    drawScene();
    
    // calculate ambient occlusion
    if (m_occlusionOptions->halfResolution())
    {
        glViewport(
            m_viewportCapability->x(),
            m_viewportCapability->y(),
            m_viewportCapability->width() / 2,
            m_viewportCapability->height() / 2);
    }

    m_ambientOcclusionStage->process(m_normalDepthAttachment);
    
    // blur ambient occlusion texture
    glViewport(
        m_viewportCapability->x(),
        m_viewportCapability->y(),
        m_viewportCapability->width(),
        m_viewportCapability->height());

    auto occlusionTexture = m_ambientOcclusionStage->getOcclusionTexture();
    blur(occlusionTexture, m_normalDepthAttachment, m_blurFbo);
    
    // finally, render to screen
    auto default_framebuffer = m_targetFramebufferCapability->framebuffer();
    if (!default_framebuffer) {
        default_framebuffer = globjects::Framebuffer::defaultFBO();
    }
    
    default_framebuffer->bind(GL_FRAMEBUFFER);
    default_framebuffer->clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glEnable(GL_DEPTH_TEST);
    
    m_screenAlignedQuad->setProgram(m_mixProgram);
    
    m_screenAlignedQuad->setTextures({
        { "u_color", m_colorAttachment },
        { "u_blur", m_blurAttachment },
        { "u_normal_depth", m_depthBuffer }
    });
    
    m_screenAlignedQuad->draw();

    glDisable(GL_DEPTH_TEST);
}

void AmbientOcclusion::blur(globjects::Texture *input, globjects::Texture *normals, globjects::Framebuffer *output)
{
    // pass 1 (x)
    m_blurTmpFbo->bind();
    m_blurTmpFbo->clearBuffer(GL_COLOR, 0, glm::vec4{ 0.0, 0.0, 0.0, 0.0 });

    m_screenAlignedQuad->setProgram(m_blurXProgram);
    m_screenAlignedQuad->setTextures({
        { "u_occlusion", input },
        { "u_normal_depth", normals }
    });
    m_screenAlignedQuad->setUniforms(
        "u_kernelSize", m_occlusionOptions->blurKernelSize(),
        "u_biliteral", m_occlusionOptions->biliteralBlurring()
    );
    m_screenAlignedQuad->draw();

    // pass 2 (y)
    output->bind();
    output->clearBuffer(GL_COLOR, 0, glm::vec4{ 0.0, 0.0, 0.0, 0.0 });

    m_screenAlignedQuad->setProgram(m_blurYProgram);
    m_screenAlignedQuad->setTextures({
        { "u_occlusion", m_blurTmpAttachment },
        { "u_normal_depth", normals }
    });
    m_screenAlignedQuad->setUniforms(
        "u_kernelSize", m_occlusionOptions->blurKernelSize(),
        "u_biliteral", m_occlusionOptions->biliteralBlurring()
    );
    m_screenAlignedQuad->draw();
}
