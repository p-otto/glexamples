#pragma once

#include "AmbientOcclusionStage.h"

class HBAO : public AmbientOcclusionStage
{
public:
    HBAO(const AmbientOcclusionOptions *options);
    virtual ~HBAO() = default;

    virtual std::vector<glm::vec3> getKernel(int size) override;
    virtual std::vector<float> getSamplingDirections(int size);
    virtual std::vector<glm::vec3> getNoiseTexture(int size) override;

    virtual void process(globjects::Texture *normalsDepth, globjects::Texture * color) override;
    virtual void setupKernel() override;

protected:
    std::vector<float> m_samplingDirections;
};
