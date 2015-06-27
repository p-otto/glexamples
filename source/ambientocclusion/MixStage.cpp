#include "MixStage.h"

#include "ScreenAlignedQuadRenderer.h"
#include "AmbientOcclusionOptions.h"

#include <glbinding/gl/enum.h>
#include <glbinding/gl/bitfield.h>
#include <glbinding/gl/functions.h>

#include <globjects/Texture.h>
#include <globjects/Framebuffer.h>
#include <globjects/Program.h>
#include <globjects/Shader.h>

#include <gloperate/base/make_unique.hpp>

using namespace gl;
using namespace globjects;

MixStage::MixStage(const AmbientOcclusionOptions * options) 
:   m_occlusionOptions(options)
{}

void MixStage::initialize()
{
	m_screenAlignedQuad = gloperate::make_unique<ScreenAlignedQuadRenderer>();

	m_mixProgram = new Program{};
	m_mixProgram->attach(
		Shader::fromFile(GL_VERTEX_SHADER, "data/ambientocclusion/screen_quad.vert"),
		Shader::fromFile(GL_FRAGMENT_SHADER, "data/ambientocclusion/mix.frag")
    );
}

void MixStage::process(globjects::Texture *colorTexture, globjects::Texture *blurTexture, globjects::Texture *normalDepthTexture, globjects::Texture *depthBuffer)
{
	m_screenAlignedQuad->setProgram(m_mixProgram);
	m_screenAlignedQuad->setTextures({
		{ "u_color", colorTexture },
		{ "u_blur", blurTexture },
		{ "u_normal_depth", normalDepthTexture },
        { "u_depth", depthBuffer }
	});

	m_screenAlignedQuad->draw();

}

MixStage::~MixStage()
{
}
