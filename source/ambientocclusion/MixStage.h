#pragma once

#include <vector>
#include <memory>
#include <globjects/base/ref_ptr.h>

class AmbientOcclusionOptions;
class ScreenAlignedQuadRenderer;

namespace globjects
{
	class Framebuffer;
	class Program;
	class Texture;
}

class MixStage
{
public:
	MixStage(const AmbientOcclusionOptions * options);
	virtual ~MixStage();

	void initialize();
	void process(globjects::Texture *colorTexture, globjects::Texture *blurTexture, globjects::Texture *normalDepthTexture);

	void updateFramebuffer(const int width, const int height);

	globjects::Texture * getMixedTexture();

protected:
	const AmbientOcclusionOptions * m_occlusionOptions;

	globjects::ref_ptr<globjects::Framebuffer> m_mixFbo;
	globjects::ref_ptr<globjects::Program> m_mixProgram;
	globjects::ref_ptr<globjects::Texture> m_mixAttachment;

	std::unique_ptr<ScreenAlignedQuadRenderer> m_screenAlignedQuad;
};

