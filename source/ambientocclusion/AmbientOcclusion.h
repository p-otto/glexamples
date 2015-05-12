#pragma once

#include <memory>

#include <glbinding/gl/types.h>

#include <globjects/base/ref_ptr.h>

#include <gloperate/painter/Painter.h>

#include <glm/glm.hpp>

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
    
	void blur(globjects::Texture *input, globjects::Framebuffer *output);

    std::vector<glm::vec3> getNormalOrientedKernel(int size);
    std::vector<glm::vec3> getNormalOrientedRotationTexture(int size);
    
    std::vector<glm::vec3> getCrytekKernel(int size);
    std::vector<glm::vec3> getCrytekReflectionTexture(int size);

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
    globjects::ref_ptr<globjects::Framebuffer> m_modelFbo;
    globjects::ref_ptr<globjects::Texture> m_colorAttachment;
    globjects::ref_ptr<globjects::Texture> m_normalDepthAttachment;
    globjects::ref_ptr<globjects::Texture> m_depthBuffer;
    
    globjects::ref_ptr<globjects::Framebuffer> m_occlusionFbo;
    globjects::ref_ptr<globjects::Texture> m_occlusionAttachment;
    
    globjects::ref_ptr<globjects::Framebuffer> m_blurFbo;
    globjects::ref_ptr<globjects::Texture> m_blurAttachment;
    
    globjects::ref_ptr<globjects::Program> m_modelProgram;
    globjects::ref_ptr<globjects::Program> m_ambientOcclusionProgram;
    globjects::ref_ptr<globjects::Program> m_blurProgram;
    globjects::ref_ptr<globjects::Program> m_mixProgram;
    
    std::vector<glm::vec3> m_kernel;
    globjects::ref_ptr<globjects::Texture> m_rotationTex;
    
    globjects::ref_ptr<gloperate::AdaptiveGrid> m_grid;
    std::unique_ptr<gloperate::PolygonalDrawable> m_model;
    globjects::ref_ptr<gloperate::ScreenAlignedQuad> m_screenAlignedQuad;
    
    bool m_normalOriented = true;
    int m_rotationTexSize = 4;
    int m_kernelSize = 32;
    float m_minimalKernelLength = 0.1f;
    float m_minimalKernelAngle = 0.1f;
};
