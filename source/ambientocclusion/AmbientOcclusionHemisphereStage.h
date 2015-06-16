#pragma once

#include "AbstractAmbientOcclusionStage.h"

#include <vector>

#include <globjects/base/ref_ptr.h>

#include <glm/glm.hpp>

class AmbientOcclusionOptions;

class AmbientOcclusionHemisphereStage : public AbstractAmbientOcclusionStage
{
public:
    AmbientOcclusionHemisphereStage(const AmbientOcclusionOptions *options);
    virtual ~AmbientOcclusionHemisphereStage() = default;

protected:
    virtual void initializeShaders() override;
    virtual std::vector<glm::vec3> getKernel(int size) override;
    virtual std::vector<glm::vec3> getNoiseTexture(int size) override;
};
