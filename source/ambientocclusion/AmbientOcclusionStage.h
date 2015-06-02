#pragma once

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

    void initialize();
    void process(globjects::Texture *normalsDepth);

    globjects::Texture * getOcclusionTexture();
    gloperate::UniformGroup * getUniformGroup();

    void setupKernelAndRotationTex();
    void updateFramebuffer(const int width, const int height);

protected:
    std::vector<glm::vec3> getHemisphereKernel(int size);
    std::vector<glm::vec3> getRotationTexture(int size);

    std::vector<glm::vec3> getSphereKernel(int size);
    std::vector<glm::vec3> getReflectionTexture(int size);

    globjects::ref_ptr<globjects::Framebuffer> m_occlusionFbo;
    globjects::ref_ptr<globjects::Texture> m_occlusionAttachment;

    globjects::ref_ptr<globjects::Program> m_ambientOcclusionProgramNormalOriented;
    globjects::ref_ptr<globjects::Program> m_ambientOcclusionProgramCrytek;

    const AmbientOcclusionOptions * m_occlusionOptions;

    globjects::ref_ptr<globjects::Texture> m_rotationTex;
    std::vector<glm::vec3> m_kernel;

    std::unique_ptr<std::default_random_engine> m_randEngine;
    std::unique_ptr<gloperate::UniformGroup> m_uniformGroup;

    std::unique_ptr<ScreenAlignedQuadRenderer> m_screenAlignedQuad;
};
