#pragma once

#include "AmbientOcclusionStage.h"

class SSAOSphere : public AmbientOcclusionStage
{
public:
    SSAOSphere(const AmbientOcclusionOptions *options);
    virtual ~SSAOSphere() = default;

protected:
    virtual std::vector<glm::vec3> getKernel(int size) override;
    virtual std::vector<glm::vec3> getNoiseTexture(int size) override;
};
