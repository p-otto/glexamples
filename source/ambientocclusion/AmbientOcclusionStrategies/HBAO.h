#pragma once

#include "AmbientOcclusionStrategy.h"

class AmbientOcclusionOptions;

class HBAO : public AmbientOcclusionStrategy
{
public:
    HBAO(const AmbientOcclusionOptions *options);
    virtual ~HBAO() = default;

    virtual std::vector<glm::vec3> getKernel(int size) override;
    virtual std::vector<glm::vec3> getNoiseTexture(int size) override;
};
