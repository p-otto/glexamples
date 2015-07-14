#pragma once

#include "AmbientOcclusionStage.h"

class SSAOHemisphere : public AmbientOcclusionStage
{
public:
    SSAOHemisphere(const AmbientOcclusionOptions * options);
    virtual ~SSAOHemisphere() = default;

protected:
    virtual Kernel::KernelType getKernelType() override;
    virtual std::vector<glm::vec3> getNoiseTexture(int size) override;
};
