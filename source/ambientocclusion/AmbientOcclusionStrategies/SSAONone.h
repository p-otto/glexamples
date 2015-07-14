#pragma once

#include "AmbientOcclusionStage.h"

class SSAONone : public AmbientOcclusionStage
{
public:
    SSAONone(const AmbientOcclusionOptions *options);
    virtual ~SSAONone() = default;

protected:
    virtual Kernel::KernelType getKernelType() override;
    virtual std::vector<glm::vec3> getNoiseTexture(int size) override;
};
