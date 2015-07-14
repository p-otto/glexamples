#include "SSDO.h"

#include "ScreenAlignedQuadRenderer.h"

#include <glbinding/gl/enum.h>
#include <glbinding/gl/bitfield.h>
#include <glbinding/gl/functions.h>

#include <globjects/Texture.h>
#include <globjects/Framebuffer.h>
#include <globjects/Program.h>
#include <globjects/Shader.h>

#include <gloperate/primitives/UniformGroup.h>
#include <gloperate/base/make_unique.hpp>

#include <glm/gtc/type_ptr.hpp>

using namespace gl;
using namespace globjects;

SSDO::SSDO(const AmbientOcclusionOptions * options)
:   AmbientOcclusionStage(options)
{}

void SSDO::initializeMethodSpecific()
{
    m_program = new Program{};
    m_program->attach(
        Shader::fromFile(GL_VERTEX_SHADER, "data/ambientocclusion/screen_quad.vert"),
        Shader::fromFile(GL_FRAGMENT_SHADER, "data/ambientocclusion/ssdo_bounce.frag")
    );

    m_directLightingShader = new Program{};
    m_directLightingShader->attach(
        Shader::fromFile(GL_VERTEX_SHADER, "data/ambientocclusion/screen_quad.vert"),
        Shader::fromFile(GL_FRAGMENT_SHADER, "data/ambientocclusion/ssdo_direct.frag")
    );

    m_firstPassAttachment = Texture::createDefault(GL_TEXTURE_2D);

    m_firstPassFbo = new Framebuffer{};
    m_firstPassFbo->attachTexture(GL_COLOR_ATTACHMENT0, m_firstPassAttachment);
}

void SSDO::updateFramebufferMethodSpecific(const int width, const int height)
{
    m_firstPassAttachment->image2D(0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
}

Kernel::KernelType SSDO::getKernelType()
{
    return Kernel::KernelType::Hemisphere;
}

std::vector<glm::vec3> SSDO::getNoiseTexture(int size)
{
    std::vector<glm::vec3> tex(size * size);

    std::uniform_real_distribution<float> distribution(-1.0, 1.0);

    for (auto &vec : tex)
    {
        vec[0] = distribution(m_randEngine);
        vec[1] = distribution(m_randEngine);
        vec[2] = 0.0f;

        vec = glm::normalize(vec);
    }

    return tex;
}

void SSDO::process(globjects::Texture * normalsDepth, std::vector<globjects::Texture*> colors)
{
    // first pass: direct lighting
    m_screenAlignedQuad->setProgram(m_directLightingShader);

    m_firstPassFbo->bind();
    m_firstPassFbo->clearBuffer(GL_COLOR, 0, glm::vec4{ 0.0, 0.0, 0.0, 0.0 });

    m_screenAlignedQuad->setTextures({
        { "u_normal_depth", normalsDepth },
        { "u_rotation", m_rotationTex },
    });

    m_uniformGroup->addToProgram(m_screenAlignedQuad->program());

    glProgramUniform3fv(
        m_screenAlignedQuad->program()->id(),
        m_screenAlignedQuad->program()->getUniformLocation("kernel"),
        m_occlusionOptions->kernelSize(),
        glm::value_ptr((m_kernel)[0])
    );

    m_screenAlignedQuad->draw();

    // second pass: bounce
    m_screenAlignedQuad->setProgram(m_program);

    m_occlusionFbo->bind();
    m_occlusionFbo->clearBuffer(GL_COLOR, 0, glm::vec4{ 0.0, 0.0, 0.0, 0.0 });

    m_screenAlignedQuad->setTextures({
        { "u_normal_depth", normalsDepth },
        { "u_rotation", m_rotationTex },
        { "u_direct_light", m_firstPassAttachment },
        { "u_ambient", colors.at(0) },
        { "u_diffuse", colors.at(1) }
    });

    glProgramUniform3fv(
        m_screenAlignedQuad->program()->id(),
        m_screenAlignedQuad->program()->getUniformLocation("kernel"),
        m_occlusionOptions->kernelSize(),
        glm::value_ptr((m_kernel)[0])
    );

    m_uniformGroup->addToProgram(m_screenAlignedQuad->program());

    m_screenAlignedQuad->draw();
}
