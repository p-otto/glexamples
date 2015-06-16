#pragma once

#include "AbstractAmbientOcclusionStage.h"

class AmbientOcclusionOptions;

class AmbientOcclusionSphereStage : public AbstractAmbientOcclusionStage
{
public:
    AmbientOcclusionSphereStage(const AmbientOcclusionOptions *options);
    virtual ~AmbientOcclusionSphereStage() = default;

protected:
    virtual void initializeShaders() override;
    virtual std::vector<glm::vec3> getKernel(int size) override;
    virtual std::vector<glm::vec3> getNoiseTexture(int size) override;
};
