#pragma once

#include <vector>
#include <memory>

#include <globjects/base/ref_ptr.h>

class ScreenAlignedQuadRenderer;
class AmbientOcclusionOptions;

namespace globjects
{
    class Framebuffer;
    class Program;
    class Texture;
}

namespace gloperate
{
    class UniformGroup;
}

class BlurStage
{
public:
    BlurStage(const AmbientOcclusionOptions * options);
    ~BlurStage() = default;
    
    void initialize();
    void process(globjects::Texture *input, globjects::Texture *normals);

    globjects::Texture * getBlurredTexture();

    void updateFramebuffer(const int width, const int height);
    
protected:
    globjects::ref_ptr<globjects::Framebuffer> m_blurFbo;
    globjects::ref_ptr<globjects::Framebuffer> m_blurTmpFbo;
    globjects::ref_ptr<globjects::Texture> m_blurAttachment;
    globjects::ref_ptr<globjects::Texture> m_blurTmpAttachment;

    globjects::ref_ptr<globjects::Program> m_blurXProgram;
    globjects::ref_ptr<globjects::Program> m_blurYProgram;

    std::unique_ptr<ScreenAlignedQuadRenderer> m_screenAlignedQuad;

    const AmbientOcclusionOptions * m_occlusionOptions;
};
