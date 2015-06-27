#pragma once

#include "AmbientOcclusionOptions.h"

#include <vector>
#include <memory>
#include <random>

#include <globjects/base/ref_ptr.h>

#include <glm/glm.hpp>

class AmbientOcclusionOptions;
class ScreenAlignedQuadRenderer;

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
    virtual ~AmbientOcclusionStage() = default;

    void initialize();
    virtual void process(globjects::Texture * normalsDepth, globjects::Texture * color);

    globjects::Texture * getOcclusionTexture();
    gloperate::UniformGroup * getUniformGroup();

    void setupKernelAndRotationTex();
    void updateFramebuffer(const int width, const int height);

protected:
    virtual std::vector<glm::vec3> getKernel(int size) = 0;
    virtual std::vector<glm::vec3> getNoiseTexture(int size) = 0;
    virtual void initializeMethodSpecific();
    virtual void updateFramebufferMethodSpecific(const int width, const int height);

    /* members */
    const AmbientOcclusionOptions * m_occlusionOptions;

    globjects::ref_ptr<globjects::Framebuffer> m_occlusionFbo;
    globjects::ref_ptr<globjects::Texture> m_occlusionAttachment;

    globjects::ref_ptr<globjects::Texture> m_rotationTex;
    std::vector<glm::vec3> m_kernel;

    globjects::ref_ptr<globjects::Program> m_program;

    std::unique_ptr<gloperate::UniformGroup> m_uniformGroup;
    std::default_random_engine m_randEngine;

    std::unique_ptr<ScreenAlignedQuadRenderer> m_screenAlignedQuad;
};
