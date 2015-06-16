#pragma once

#include <memory>

#include <glbinding/gl/types.h>

#include <globjects/base/ref_ptr.h>

#include <gloperate/painter/Painter.h>

class ScreenAlignedQuadRenderer;
class AmbientOcclusionOptions;

class AmbientOcclusionStage;
class BlurStage;
class GeometryStage;
class MixStage;

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
}

class AmbientOcclusion : public gloperate::Painter
{
public:
    AmbientOcclusion(gloperate::ResourceManager & resourceManager);
    virtual ~AmbientOcclusion();

    void setupProjection();
    void updateFramebuffers();
    void setupKernelAndRotationTex();

    void drawGrid();
    void drawGeometry();
    void drawScreenSpaceAmbientOcclusion();
    void drawWithoutAmbientOcclusion();

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
    std::unique_ptr<BlurStage> m_blurStage;
    std::unique_ptr<GeometryStage> m_geometryStage;
    std::unique_ptr<MixStage> m_mixStage;
    
    globjects::ref_ptr<globjects::Program> m_mixProgram;
    
    globjects::ref_ptr<gloperate::AdaptiveGrid> m_grid;
    std::unique_ptr<ScreenAlignedQuadRenderer> m_screenAlignedQuad;
};
