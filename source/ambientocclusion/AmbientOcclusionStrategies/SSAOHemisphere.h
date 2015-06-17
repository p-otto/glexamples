#pragma once

#include "AmbientOcclusionStrategy.h"

class SSAOHemisphere : public AmbientOcclusionStrategy
{
public:
    SSAOHemisphere(const AmbientOcclusionOptions * options);
    virtual ~SSAOHemisphere() = default;

    virtual std::vector<glm::vec3> getKernel(int size) override;
    virtual std::vector<glm::vec3> getNoiseTexture(int size) override;
};
