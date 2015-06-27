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
        Shader::fromFile(GL_FRAGMENT_SHADER, "data/ambientocclusion/ssdo_direct.frag", { "/lights" })
    );

    m_firstPassAttachment = Texture::createDefault(GL_TEXTURE_2D);

    m_firstPassFbo = new Framebuffer{};
    m_firstPassFbo->attachTexture(GL_COLOR_ATTACHMENT0, m_firstPassAttachment);
}

void SSDO::updateFramebufferMethodSpecific(const int width, const int height)
{
    m_firstPassAttachment->image2D(0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
}

std::vector<glm::vec3> SSDO::getKernel(int size)
{
    std::vector<glm::vec3> kernel(size);
    int count = 1;

    std::uniform_real_distribution<float> distribution(-1.0, 1.0);
    std::uniform_real_distribution<float> positive_distribution(0.0, 1.0);

    for (auto &vec : kernel)
    {
        do {
            vec[0] = distribution(m_randEngine);
            vec[1] = distribution(m_randEngine);
            vec[2] = positive_distribution(m_randEngine);
        } while (glm::dot(vec, glm::vec3(0, 0, 1)) < m_occlusionOptions->minimalKernelAngle());

        vec = glm::normalize(vec);

        float scale = static_cast<float>(count++) / size;
        scale = glm::mix(m_occlusionOptions->minimalKernelLength(), 1.0f, scale * scale);
        vec *= scale;
    }

    return kernel;
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

void SSDO::process(globjects::Texture * normalsDepth, globjects::Texture * color)
{
    // first pass: direct lighting
    m_screenAlignedQuad->setProgram(m_directLightingShader);

    m_firstPassFbo->bind();
    m_firstPassFbo->clearBuffer(GL_COLOR, 0, glm::vec4{ 0.0, 0.0, 0.0, 0.0 });

    m_screenAlignedQuad->setTextures({
        { "u_normal_depth", normalsDepth },
        { "u_rotation", m_rotationTex },
        { "u_color", color }
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
        { "u_direct_light", m_firstPassAttachment }
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
