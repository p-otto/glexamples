#pragma once

#include "AbstractAmbientOcclusionStage.h"

#include <vector>

#include <glm/glm.hpp>

class AmbientOcclusionOptions;

class AmbientOcclusionNoneStage : public AbstractAmbientOcclusionStage
{
public:
    AmbientOcclusionNoneStage(const AmbientOcclusionOptions *options);
    virtual ~AmbientOcclusionNoneStage() = default;

protected:
    virtual void initializeShaders() override;
    virtual std::vector<glm::vec3> getKernel(int size) override;
    virtual std::vector<glm::vec3> getNoiseTexture(int size) override;
};
