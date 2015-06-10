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
	: m_occlusionOptions(options)
{}

void MixStage::initialize()
{
	m_screenAlignedQuad = gloperate::make_unique<ScreenAlignedQuadRenderer>();

	m_mixProgram = new Program{};
	m_mixProgram->attach(
		Shader::fromFile(GL_VERTEX_SHADER, "data/ambientocclusion/screen_quad.vert"),
		Shader::fromFile(GL_FRAGMENT_SHADER, "data/ambientocclusion/mix.frag")
		);

	m_mixAttachment = Texture::createDefault(GL_TEXTURE_2D);

	m_mixFbo = make_ref<Framebuffer>();
	m_mixFbo->attachTexture(GL_COLOR_ATTACHMENT0, m_mixAttachment);
}

void MixStage::process(globjects::Texture *colorTexture, globjects::Texture *blurTexture, globjects::Texture *normalDepthTexture)
{
	m_mixFbo->bind();
	m_mixFbo->clearBuffer(GL_COLOR, 0, glm::vec4{ 0.0, 0.0, 0.0, 0.0 });

	m_screenAlignedQuad->setProgram(m_mixProgram);
	m_screenAlignedQuad->setTextures({
		{ "u_color", colorTexture },
		{ "u_blur", blurTexture },
		{ "u_normal_depth", normalDepthTexture }
	});

	m_screenAlignedQuad->draw();

}

void MixStage::updateFramebuffer(const int width, const int height)
{
	m_mixAttachment->image2D(0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
}

globjects::Texture * MixStage::getMixedTexture()
{
	return m_mixAttachment;
}


MixStage::~MixStage()
{
}
