#include "AmbientOcclusion.h"

#include "Plane.h"
#include "ScreenAlignedQuadRenderer.h"
#include "UniformHelper.h"

#include "BlurStage.h"
#include "AmbientOcclusionStage.h"
#include "GeometryStage.h"
#include "MixStage.h"

#include "AmbientOcclusionStrategies/SSAONone.h"
#include "AmbientOcclusionStrategies/SSAOHemisphere.h"
#include "AmbientOcclusionStrategies/SSAOSphere.h"
#include "AmbientOcclusionStrategies/HBAO.h"
#include "AmbientOcclusionStrategies/SSDO.h"

#include <chrono>
#include <fstream>

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
#include <globjects/NamedString.h>

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

AmbientOcclusion::AmbientOcclusion(gloperate::ResourceManager & resourceManager, const std::string & relDataPath)
:   Painter("AmbientOcclusion", resourceManager, relDataPath)
,   m_targetFramebufferCapability(addCapability(new gloperate::TargetFramebufferCapability()))
,   m_viewportCapability(addCapability(new gloperate::ViewportCapability()))
,   m_projectionCapability(addCapability(new gloperate::PerspectiveProjectionCapability(m_viewportCapability)))
,   m_cameraCapability(addCapability(new gloperate::CameraCapability()))
,   m_occlusionOptions(new AmbientOcclusionOptions(*this))
,   m_geometryStage(gloperate::make_unique<GeometryStage>(m_occlusionOptions.get()))
,   m_blurStage(gloperate::make_unique<BlurStage>(m_occlusionOptions.get()))
,   m_mixStage(gloperate::make_unique<MixStage>(m_occlusionOptions.get()))
{}

AmbientOcclusion::~AmbientOcclusion() = default;

void AmbientOcclusion::setupProjection()
{
    static const auto zNear = 0.3f, zFar = 500.f, fovy = 50.f;

    m_projectionCapability->setZNear(zNear);
    m_projectionCapability->setZFar(zFar);
    m_projectionCapability->setFovy(radians(fovy));

    m_grid->setNearFar(zNear, zFar);
}

void AmbientOcclusion::updateAmbientOcclusion()
{
    switch (m_occlusionOptions->ambientOcclusion())
    {
    case ScreenSpaceSphere:
        m_ambientOcclusionStage = gloperate::make_unique<SSAOSphere>(m_occlusionOptions.get());
        break;
    case ScreenSpaceHemisphere:
        m_ambientOcclusionStage = gloperate::make_unique<SSAOHemisphere>(m_occlusionOptions.get());
        break;
    case ScreenSpaceDirectional:
        m_ambientOcclusionStage = gloperate::make_unique<SSDO>(m_occlusionOptions.get());
        break;
    case HorizonBased:
        m_ambientOcclusionStage = gloperate::make_unique<HBAO>(m_occlusionOptions.get());
        break;
    default:
        m_ambientOcclusionStage = gloperate::make_unique<SSAONone>(m_occlusionOptions.get());
        break;
    }

    m_ambientOcclusionStage->initialize();

    const auto width = m_viewportCapability->width(), height = m_viewportCapability->height();
    m_ambientOcclusionStage->updateFramebuffer(width, height);

    m_ambientOcclusionStage->setupKernel();
    m_ambientOcclusionStage->setupRotationTex();
}

void AmbientOcclusion::updateFramebuffers()
{
    const auto width = m_viewportCapability->width(), height = m_viewportCapability->height();

    m_geometryStage->updateFramebuffer(width, height);
    m_ambientOcclusionStage->updateFramebuffer(width, height);
    m_blurStage->updateFramebuffer(width, height);
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

    std::ifstream t("data/ambientocclusion/lights.glsl");
    std::stringstream buffer;
    buffer << t.rdbuf();
    NamedString::create("/lights", buffer.str());

    t = std::ifstream("data/ambientocclusion/utility.glsl");
    buffer << t.rdbuf();
    NamedString::create("/utility", buffer.str());

    m_grid = make_ref<gloperate::AdaptiveGrid>();
    m_grid->setColor({0.6f, 0.6f, 0.6f});

    m_cameraCapability->setEye(glm::vec3(0.0f, 30.0f, -120.0f));
    m_cameraCapability->setCenter(glm::vec3(0.2f, 30.3f, 0.0f));

    auto scene = m_resourceManager.load<gloperate::Scene>("data/ambientocclusion/scifiroom/Scifi.3DS");

    updateAmbientOcclusion();

    m_blurStage->initialize();
    m_geometryStage->initialize(scene);
    m_mixStage->initialize();
    
    updateFramebuffers();
    setupProjection();
}

void AmbientOcclusion::updateKernel() {
	m_ambientOcclusionStage->setupKernel();
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
    
    if (m_occlusionOptions->hasAmbientOcclusionTypeChanged()) {
        updateAmbientOcclusion();
    }

	if (m_occlusionOptions->hasKernelChanged()) {
		updateKernel();
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
    model = glm::translate(model, glm::vec3(20, 0, 0));
    model = glm::scale(model, glm::vec3(0.1f));
    model = glm::rotate(model, 3.14f, glm::vec3(0, 0, 1));
    model = glm::rotate(model, 3.14f / 2.0f, glm::vec3(1, 0, 0));
    setUniforms(*m_geometryStage->getUniformGroup(),
        "u_mvp", m_projectionCapability->projection() * m_cameraCapability->view() * model,
        "u_modelView", m_cameraCapability->view() * model,
        "u_model", model,
        "u_farPlane", m_projectionCapability->zFar(),
        "u_nearPlane", m_projectionCapability->zNear()
    );
    m_geometryStage->process();

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
}

void AmbientOcclusion::drawScreenSpaceAmbientOcclusion()
{
    // draw geometry to texture
    drawGeometry();

    glDisable(GL_DEPTH_TEST);

    int width = m_viewportCapability->width();
    int height = m_viewportCapability->height();
    
    // calculate ambient occlusion
    if (m_occlusionOptions->halfResolution())
    {
        width /= 2;
        height /= 2;
        glViewport(
            m_viewportCapability->x(),
            m_viewportCapability->y(),
            width,
            height);
    }

    setUniforms(*m_ambientOcclusionStage->getUniformGroup(),
        "u_invProj", glm::inverse(m_projectionCapability->projection()),
        "u_proj", m_projectionCapability->projection(),
        "u_view", m_cameraCapability->view(),
        "u_nearPlane", m_projectionCapability->zNear(),
        "u_farPlane", m_projectionCapability->zFar(),
        "u_resolutionX", width,
        "u_resolutionY", height,
        "u_kernelSize", m_occlusionOptions->kernelSize(),
        "u_kernelRadius", m_occlusionOptions->kernelRadius(),
        "u_numSamples", m_occlusionOptions->numSamples(),
        "u_numDirections", m_occlusionOptions->numDirections(),
        "u_lengthDistribution", (int)m_occlusionOptions->lengthDistribution(),
        "u_color_bleeding_strength", m_occlusionOptions->colorBleedingStrength()
    );
    auto ambientTexture = m_geometryStage->getAmbientTexture();
    auto diffuseTexture = m_geometryStage->getDiffuseTexture();
    auto normalDepthTexture = m_geometryStage->getNormalDepthTexture();

    m_ambientOcclusionStage->process(normalDepthTexture, { ambientTexture, diffuseTexture });

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

    glEnable(GL_DEPTH_TEST);

    default_framebuffer->bind(GL_FRAMEBUFFER);
    default_framebuffer->clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto blurTexture = m_blurStage->getBlurredTexture();
    auto depthBuffer = m_geometryStage->getDepthBuffer();
    m_mixStage->process(ambientTexture, diffuseTexture, blurTexture, normalDepthTexture, depthBuffer);
}
