#pragma once

#include "AmbientOcclusionStrategy.h"

class AmbientOcclusionOptions;

class SSAOSphere : public AmbientOcclusionStrategy
{
public:
    SSAOSphere(const AmbientOcclusionOptions *options);
    virtual ~SSAOSphere() = default;

    virtual std::vector<glm::vec3> getKernel(int size) override;
    virtual std::vector<glm::vec3> getNoiseTexture(int size) override;
};
