#pragma once

#include "AmbientOcclusionStage.h"
#include "Kernel.h"

class HBAO : public AmbientOcclusionStage
{
public:
    HBAO(const AmbientOcclusionOptions *options);
    virtual ~HBAO() = default;

    virtual Kernel::KernelType getKernelType() override;
    virtual std::vector<glm::vec3> getNoiseTexture(int size) override;


protected:
    std::vector<float> m_samplingDirections;
};
