#pragma once
#include "AmbientOcclusionStage.h"
class SSAOGeneral : public AmbientOcclusionStage
{
public:
	SSAOGeneral(const AmbientOcclusionOptions * options);
	virtual ~SSAOGeneral() = default;

protected:
	virtual std::vector<glm::vec3> getKernel(int size) override;
	virtual std::vector<glm::vec3> getNoiseTexture(int size) override;
};

