#pragma once

#include <vector>
#include <memory>

#include <globjects/base/ref_ptr.h>

class AmbientOcclusionOptions;
class Plane;

namespace globjects
{
    class Framebuffer;
    class Program;
    class Texture;
}

namespace gloperate
{
    class PolygonalDrawable;
    class Scene;
    class UniformGroup;
};

class GeometryStage
{
public:
    GeometryStage(const AmbientOcclusionOptions * options);
    ~GeometryStage() = default;

    void initialize(const gloperate::Scene * scene);
    void process();

    void bindAndClearFbo();

    globjects::Framebuffer * getFramebuffer();
    globjects::Texture * getColorTexture();
    globjects::Texture * getNormalDepthTexture();

    gloperate::UniformGroup * getUniformGroup();

    void updateFramebuffer(const int width, const int height);

private:
    const AmbientOcclusionOptions * m_occlusionOptions;

    globjects::ref_ptr<globjects::Framebuffer> m_modelFbo;
    globjects::ref_ptr<globjects::Texture> m_colorAttachment;
    globjects::ref_ptr<globjects::Texture> m_normalDepthAttachment;
    globjects::ref_ptr<globjects::Texture> m_depthBuffer;

    globjects::ref_ptr<globjects::Program> m_modelProgram;
    globjects::ref_ptr<globjects::Program> m_phongProgram;

    std::vector<gloperate::PolygonalDrawable> m_drawables;
    std::unique_ptr<Plane> m_plane;

    std::unique_ptr<gloperate::UniformGroup> m_uniformGroup;
};
