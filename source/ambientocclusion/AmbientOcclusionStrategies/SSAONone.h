#pragma once

#include "AmbientOcclusionStage.h"

class SSAONone : public AmbientOcclusionStage
{
public:
    SSAONone(const AmbientOcclusionOptions *options);
    virtual ~SSAONone() = default;

protected:
    virtual std::vector<glm::vec3> getKernel(int size) override;
    virtual std::vector<glm::vec3> getNoiseTexture(int size) override;
};
