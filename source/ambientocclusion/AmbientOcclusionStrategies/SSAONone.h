#pragma once

#include "AmbientOcclusionStrategy.h"

class AmbientOcclusionOptions;

class SSAONone : public AmbientOcclusionStrategy
{
public:
    SSAONone(const AmbientOcclusionOptions *options);
    virtual ~SSAONone() = default;

    virtual std::vector<glm::vec3> getKernel(int size) override;
    virtual std::vector<glm::vec3> getNoiseTexture(int size) override;
};
