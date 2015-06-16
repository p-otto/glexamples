#include "AmbientOcclusion.h"

#include "Plane.h"
#include "ScreenAlignedQuadRenderer.h"
#include "UniformHelper.h"

#include "AmbientOcclusionHemisphereStage.h"
#include "AmbientOcclusionSphereStage.h"
#include "AmbientOcclusionNoneStage.h"
#include "BlurStage.h"
#include "GeometryStage.h"
#include "MixStage.h"

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
,   m_ambientOcclusionNoneStage(gloperate::make_unique<AmbientOcclusionNoneStage>(m_occlusionOptions.get()))
,   m_ambientOcclusionSphereStage(gloperate::make_unique<AmbientOcclusionSphereStage>(m_occlusionOptions.get()))
,   m_ambientOcclusionHemisphereStage(gloperate::make_unique<AmbientOcclusionHemisphereStage>(m_occlusionOptions.get()))
,   m_ambientOcclusionStage(m_ambientOcclusionHemisphereStage.get())
,   m_geometryStage(gloperate::make_unique<GeometryStage>(m_occlusionOptions.get()))
,   m_blurStage(gloperate::make_unique<BlurStage>(m_occlusionOptions.get()))
,   m_mixStage(gloperate::make_unique<MixStage>(m_occlusionOptions.get()))
{}

AmbientOcclusion::~AmbientOcclusion() = default;

void AmbientOcclusion::setupProjection()
{
    static const auto zNear = 0.3f, zFar = 150.f, fovy = 50.f;

    m_projectionCapability->setZNear(zNear);
    m_projectionCapability->setZFar(zFar);
    m_projectionCapability->setFovy(radians(fovy));

    m_grid->setNearFar(zNear, zFar);
}

void AmbientOcclusion::setupKernelAndRotationTex()
{
    m_ambientOcclusionStage->setupKernelAndRotationTex();
}

void AmbientOcclusion::setAmbientOcclusion(const AmbientOcclusionType &type)
{
    switch (type) {
        case ScreenSpaceSphere:
            m_ambientOcclusionStage = m_ambientOcclusionSphereStage.get();
            break;

        case ScreenSpaceHemisphere:
            m_ambientOcclusionStage = m_ambientOcclusionHemisphereStage.get();
            break;

        default:
            m_ambientOcclusionStage = m_ambientOcclusionNoneStage.get();
            break;
    }

    updateFramebuffers();
}

void AmbientOcclusion::setupFramebuffers()
{
    const auto width = m_viewportCapability->width(), height = m_viewportCapability->height();

    m_ambientOcclusionNoneStage->updateFramebuffer(width, height);
    m_ambientOcclusionSphereStage->updateFramebuffer(width, height);
    m_ambientOcclusionHemisphereStage->updateFramebuffer(width, height);
    
    m_blurStage->updateFramebuffer(width, height);
    m_geometryStage->updateFramebuffer(width, height);
}

void AmbientOcclusion::updateFramebuffers()
{
    const auto width = m_viewportCapability->width(), height = m_viewportCapability->height();

    m_ambientOcclusionStage->updateFramebuffer(width, height);
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

    m_grid = make_ref<gloperate::AdaptiveGrid>();
    m_grid->setColor({0.6f, 0.6f, 0.6f});

    // some magic numbers that give a good view on the teapot
    m_cameraCapability->setEye(glm::vec3(0.0f, 15.7f, -15.0f));
    m_cameraCapability->setCenter(glm::vec3(0.2f, 0.3f, 0.0f));

    auto scene = m_resourceManager.load<gloperate::Scene>("data/ambientocclusion/dragon.obj");

    m_ambientOcclusionNoneStage->initialize();
    m_ambientOcclusionSphereStage->initialize();
    m_ambientOcclusionHemisphereStage->initialize();

    m_blurStage->initialize();
    m_geometryStage->initialize(scene);
    m_mixStage->initialize();
    
    setupFramebuffers();
    setupProjection();
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
    
    drawScreenSpaceAmbientOcclusion();

    drawGrid();
}

void AmbientOcclusion::drawGrid()
{
    // move grid below plane
    glm::mat4 model{};
    model = glm::translate(model, glm::vec3(0.0f, -0.1f, 0.0f));
    const auto transform = m_projectionCapability->projection() * m_cameraCapability->view() * model;
    const auto eye = m_cameraCapability->eye();
    
    glEnable(GL_DEPTH_TEST);
    
    m_grid->update(eye, transform);
    m_grid->draw();

    glDisable(GL_DEPTH_TEST);
}

void AmbientOcclusion::drawGeometry()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glm::mat4 model{};
    setUniforms(*m_geometryStage->getUniformGroup(),
        "u_mvp", m_projectionCapability->projection() * m_cameraCapability->view() * model,
        "u_modelView", m_cameraCapability->view() * model,
        "u_farPlane", m_projectionCapability->zFar()
    );
    m_geometryStage->process();

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
}

void AmbientOcclusion::drawScreenSpaceAmbientOcclusion()
{
    // draw geometry to texture
    m_geometryStage->bindAndClearFbo();
    drawGeometry();

    glDepthMask(GL_FALSE);
    glDepthFunc(GL_ALWAYS);
    
    // calculate ambient occlusion
    if (m_occlusionOptions->halfResolution())
    {
        glViewport(
            m_viewportCapability->x(),
            m_viewportCapability->y(),
            m_viewportCapability->width() / 2,
            m_viewportCapability->height() / 2);
    }

    setUniforms(*m_ambientOcclusionStage->getUniformGroup(),
        "u_invProj", glm::inverse(m_projectionCapability->projection()),
        "u_proj", m_projectionCapability->projection(),
        "u_farPlane", m_projectionCapability->zFar(),
        "u_resolutionX", m_viewportCapability->width(),
        "u_resolutionY", m_viewportCapability->height(),
        "u_kernelSize", m_occlusionOptions->kernelSize(),
        "u_kernelRadius", m_occlusionOptions->kernelRadius(),
        "u_attenuation", m_occlusionOptions->attenuation()
    );
    auto normalDepthTexture = m_geometryStage->getNormalDepthTexture();

    m_ambientOcclusionStage->process(normalDepthTexture);

    // blur ambient occlusion texture
    glViewport(
        m_viewportCapability->x(),
        m_viewportCapability->y(),
        m_viewportCapability->width(),
        m_viewportCapability->height());

    auto occlusionTexture = m_ambientOcclusionStage->getOcclusionTexture();

    m_blurStage->process(occlusionTexture, normalDepthTexture);
    
    // finally, render to screen
    auto default_framebuffer = m_targetFramebufferCapability->framebuffer();
    if (!default_framebuffer) {
        default_framebuffer = globjects::Framebuffer::defaultFBO();
    }

    default_framebuffer->bind(GL_FRAMEBUFFER);
    default_framebuffer->clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto blurTexture = m_blurStage->getBlurredTexture();
    auto colorTexture = m_geometryStage->getColorTexture();
    m_mixStage->process(colorTexture, blurTexture, normalDepthTexture);

    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
}
