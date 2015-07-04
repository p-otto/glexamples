#pragma once

#include "AmbientOcclusionStage.h"

class SSDO : public AmbientOcclusionStage
{
public:
    SSDO(const AmbientOcclusionOptions * options);
    virtual ~SSDO() = default;

    virtual void process(globjects::Texture * normalsDepth, std::vector<globjects::Texture*> colors);

protected:
    virtual std::vector<glm::vec3> getKernel(int size) override;
    virtual std::vector<glm::vec3> getNoiseTexture(int size) override;
    virtual void initializeMethodSpecific() override;
    virtual void updateFramebufferMethodSpecific(const int width, const int height);

    globjects::ref_ptr<globjects::Program> m_directLightingShader;
    globjects::ref_ptr<globjects::Framebuffer> m_firstPassFbo;
    globjects::ref_ptr<globjects::Texture> m_firstPassAttachment;
};
