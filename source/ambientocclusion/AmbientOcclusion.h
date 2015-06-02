#pragma once

#include <memory>
#include <random>

#include <glbinding/gl/types.h>

#include <globjects/base/ref_ptr.h>

#include <gloperate/painter/Painter.h>

#include <glm/glm.hpp>

class Plane;
class ScreenAlignedQuadRenderer;
class AmbientOcclusionOptions;
class AmbientOcclusionStage;

namespace globjects
{
    class Framebuffer;
    class Program;
    class Texture;
}

namespace gloperate
{
    class AdaptiveGrid;
    class Icosahedron;
    class AbstractTargetFramebufferCapability;
    class AbstractViewportCapability;
    class AbstractPerspectiveProjectionCapability;
    class AbstractCameraCapability;
    class PolygonalDrawable;
    class ScreenAlignedQuad;
}

class AmbientOcclusion : public gloperate::Painter
{
public:
    AmbientOcclusion(gloperate::ResourceManager & resourceManager);
    virtual ~AmbientOcclusion();

    void setupProjection();
    void setupFramebuffers();
    void updateFramebuffers();
    void setupModel();
    void setupShaders();
    void setupKernelAndRotationTex();

    void drawScene();
    void drawGrid();
    void drawScreenSpaceAmbientOcclusion();
    void drawWithoutAmbientOcclusion();
    void blur(globjects::Texture *input, globjects::Texture *normals, globjects::Framebuffer *output);

protected:
    virtual void onInitialize() override;
    virtual void onPaint() override;

protected:    
    /* capabilities */
    gloperate::AbstractTargetFramebufferCapability * m_targetFramebufferCapability;
    gloperate::AbstractViewportCapability * m_viewportCapability;
    gloperate::AbstractPerspectiveProjectionCapability * m_projectionCapability;
    gloperate::AbstractCameraCapability * m_cameraCapability;

    /* members */
    std::unique_ptr<AmbientOcclusionOptions> m_occlusionOptions;

    std::unique_ptr<AmbientOcclusionStage> m_ambientOcclusionStage;

    globjects::ref_ptr<globjects::Framebuffer> m_modelFbo;
    globjects::ref_ptr<globjects::Texture> m_colorAttachment;
    globjects::ref_ptr<globjects::Texture> m_normalDepthAttachment;
    globjects::ref_ptr<globjects::Texture> m_depthBuffer;
    
    globjects::ref_ptr<globjects::Framebuffer> m_blurFbo;
    globjects::ref_ptr<globjects::Framebuffer> m_blurTmpFbo;
    globjects::ref_ptr<globjects::Texture> m_blurAttachment;
    globjects::ref_ptr<globjects::Texture> m_blurTmpAttachment;
    
    globjects::ref_ptr<globjects::Program> m_modelProgram;
    globjects::ref_ptr<globjects::Program> m_phongProgram;
    globjects::ref_ptr<globjects::Program> m_blurXProgram;
    globjects::ref_ptr<globjects::Program> m_blurYProgram;
    globjects::ref_ptr<globjects::Program> m_mixProgram;
    
    globjects::ref_ptr<gloperate::AdaptiveGrid> m_grid;
    std::vector<gloperate::PolygonalDrawable> m_drawables;
    std::unique_ptr<Plane> m_plane;
    std::unique_ptr<ScreenAlignedQuadRenderer> m_screenAlignedQuad;
};
