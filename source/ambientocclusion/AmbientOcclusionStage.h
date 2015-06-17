#pragma once

#include "AmbientOcclusionOptions.h"

#include "AmbientOcclusionStrategies/SSAONone.h"
#include "AmbientOcclusionStrategies/SSAOSphere.h"
#include "AmbientOcclusionStrategies/SSAOHemisphere.h"

#include <vector>
#include <memory>
#include <random>

#include <globjects/base/ref_ptr.h>

#include <glm/glm.hpp>

class AmbientOcclusionOptions;
class ScreenAlignedQuadRenderer;

class AmbientOcclusionStrategy;
class SSAOHemisphere;
class SSAOSphere;
class SSAONone;

namespace globjects
{
    class Framebuffer;
    class Program;
    class Texture;
}

namespace gloperate
{
    class UniformGroup;
}

class AmbientOcclusionStage
{
public:
    AmbientOcclusionStage(const AmbientOcclusionOptions *options);
    ~AmbientOcclusionStage() = default;

    void initialize();
    void process(globjects::Texture *normalsDepth);

    globjects::Texture * getOcclusionTexture();
    gloperate::UniformGroup * getUniformGroup();

    void setAmbientOcclusion(const AmbientOcclusionType &type);
    void setupKernelAndRotationTex();
    void updateFramebuffer(const int width, const int height);

protected:
    /* members */
    const AmbientOcclusionOptions * m_occlusionOptions;

    AmbientOcclusionStrategy * m_strategy;
    std::unique_ptr<SSAONone> m_noSSAO;
    std::unique_ptr<SSAOSphere> m_sphereSSAO;
    std::unique_ptr<SSAOHemisphere> m_hemisphereSSAO;

    globjects::ref_ptr<globjects::Framebuffer> m_occlusionFbo;
    globjects::ref_ptr<globjects::Texture> m_occlusionAttachment;

    globjects::ref_ptr<globjects::Texture> m_rotationTex;
    std::vector<glm::vec3> m_kernel;

    std::unique_ptr<gloperate::UniformGroup> m_uniformGroup;

    std::unique_ptr<ScreenAlignedQuadRenderer> m_screenAlignedQuad;
};
