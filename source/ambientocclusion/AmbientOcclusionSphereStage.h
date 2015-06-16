#pragma once

#include "AbstractAmbientOcclusionStage.h"

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

class AmbientOcclusionSphereStage : public AbstractAmbientOcclusionStage
{
public:
    AmbientOcclusionSphereStage(const AmbientOcclusionOptions *options);
    ~AmbientOcclusionSphereStage() = default;

protected:
    virtual void initializeShaders() override;
    virtual std::vector<glm::vec3> getKernel(int size) override;
    virtual std::vector<glm::vec3> getNoiseTexture(int size) override;
};
