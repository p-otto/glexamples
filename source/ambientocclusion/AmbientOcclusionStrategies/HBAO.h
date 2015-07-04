#pragma once

#include "AmbientOcclusionStage.h"

class HBAO : public AmbientOcclusionStage
{
public:
    HBAO(const AmbientOcclusionOptions *options);
    virtual ~HBAO() = default;

    virtual std::vector<glm::vec3> getKernel(int size) override;
    virtual std::vector<glm::vec3> getNoiseTexture(int size) override;


protected:
    std::vector<float> m_samplingDirections;
};
