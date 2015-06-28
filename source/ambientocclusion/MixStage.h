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
    void process(globjects::Texture *ambientTexture, globjects::Texture *diffuseTexture, globjects::Texture *blurTexture, globjects::Texture *normalDepthTexture, globjects::Texture *depthBuffer);

protected:
	const AmbientOcclusionOptions * m_occlusionOptions;

	globjects::ref_ptr<globjects::Program> m_mixProgram;

	std::unique_ptr<ScreenAlignedQuadRenderer> m_screenAlignedQuad;
};

