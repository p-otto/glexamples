#pragma once

#include "AmbientOcclusionStage.h"

class SSAOSphere : public AmbientOcclusionStage
{
public:
    SSAOSphere(const AmbientOcclusionOptions *options);
    virtual ~SSAOSphere() = default;

protected:
    virtual Kernel::KernelType getKernelType() override;
    virtual std::vector<glm::vec3> getNoiseTexture(int size) override;
};
