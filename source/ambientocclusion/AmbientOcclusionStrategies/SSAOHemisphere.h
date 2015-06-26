#pragma once

#include "AmbientOcclusionStage.h"

class SSAOHemisphere : public AmbientOcclusionStage
{
public:
    SSAOHemisphere(const AmbientOcclusionOptions * options);
    virtual ~SSAOHemisphere() = default;

protected:
    virtual std::vector<glm::vec3> getKernel(int size) override;
    virtual std::vector<glm::vec3> getNoiseTexture(int size) override;
};
