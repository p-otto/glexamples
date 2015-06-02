#include "BlurStage.h"

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

BlurStage::BlurStage(const AmbientOcclusionOptions * options)
:   m_occlusionOptions(options)
{}

void BlurStage::initialize()
{
    m_screenAlignedQuad = gloperate::make_unique<ScreenAlignedQuadRenderer>();

    m_blurXProgram = new Program{};
    m_blurXProgram->attach(
        Shader::fromFile(GL_VERTEX_SHADER, "data/ambientocclusion/screen_quad.vert"),
        Shader::fromFile(GL_FRAGMENT_SHADER, "data/ambientocclusion/blur_x.frag")
    );

    m_blurYProgram = new Program{};
    m_blurYProgram->attach(
        Shader::fromFile(GL_VERTEX_SHADER, "data/ambientocclusion/screen_quad.vert"),
        Shader::fromFile(GL_FRAGMENT_SHADER, "data/ambientocclusion/blur_y.frag")
    );

    m_blurAttachment = Texture::createDefault(GL_TEXTURE_2D);
    m_blurTmpAttachment = Texture::createDefault(GL_TEXTURE_2D);

    m_blurFbo = make_ref<Framebuffer>();
    m_blurFbo->attachTexture(GL_COLOR_ATTACHMENT0, m_blurAttachment);
    m_blurTmpFbo = make_ref<Framebuffer>();
    m_blurTmpFbo->attachTexture(GL_COLOR_ATTACHMENT0, m_blurTmpAttachment);
}

void BlurStage::updateFramebuffer(const int width, const int height)
{
    m_blurAttachment->image2D(0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    m_blurTmpAttachment->image2D(0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
}

void BlurStage::process(globjects::Texture *input, globjects::Texture *normals)
{
    // pass 1 (x)
    m_blurTmpFbo->bind();
    m_blurTmpFbo->clearBuffer(GL_COLOR, 0, glm::vec4{ 0.0, 0.0, 0.0, 0.0 });

    m_screenAlignedQuad->setProgram(m_blurXProgram);
    m_screenAlignedQuad->setTextures({
        { "u_occlusion", input },
        { "u_normal_depth", normals }
    });
    m_screenAlignedQuad->setUniforms(
        "u_kernelSize", m_occlusionOptions->blurKernelSize(),
        "u_biliteral", m_occlusionOptions->biliteralBlurring()
    );
    m_screenAlignedQuad->draw();

    // pass 2 (y)
    m_blurFbo->bind();
    m_blurFbo->clearBuffer(GL_COLOR, 0, glm::vec4{ 0.0, 0.0, 0.0, 0.0 });

    m_screenAlignedQuad->setProgram(m_blurYProgram);
    m_screenAlignedQuad->setTextures({
        { "u_occlusion", m_blurTmpAttachment },
        { "u_normal_depth", normals }
    });
    m_screenAlignedQuad->setUniforms(
        "u_kernelSize", m_occlusionOptions->blurKernelSize(),
        "u_biliteral", m_occlusionOptions->biliteralBlurring()
    );
    m_screenAlignedQuad->draw();
}

globjects::Texture * BlurStage::getBlurredTexture()
{
    return m_blurAttachment;
}
