#include "HBAO.h"

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
#include <glm/gtc/constants.hpp>

using namespace gl;
using namespace globjects;

HBAO::HBAO(const AmbientOcclusionOptions * options)
:   AmbientOcclusionStage(options)
{
    m_program = new Program{};
    m_program->attach(
        Shader::fromFile(GL_VERTEX_SHADER, "data/ambientocclusion/screen_quad.vert"),
        Shader::fromFile(GL_FRAGMENT_SHADER, "data/ambientocclusion/hbao.frag")
    );
}

std::vector<glm::vec3> HBAO::getKernel(int size)
{
    return {};
}

std::vector<glm::vec3> HBAO::getNoiseTexture(int size)
{
    return {};
}

void HBAO::process(globjects::Texture *normalsDepth, globjects::Texture * color) {
    m_screenAlignedQuad->setProgram(m_program);

    m_occlusionFbo->bind();
    m_occlusionFbo->clearBuffer(GL_COLOR, 0, glm::vec4{ 0.0, 0.0, 0.0, 0.0 });

    m_screenAlignedQuad->setTextures({
        { "u_normal_depth", normalsDepth },
        { "u_rotation", m_rotationTex }
    });

    m_uniformGroup->addToProgram(m_screenAlignedQuad->program());

    glProgramUniform1fv(
        m_screenAlignedQuad->program()->id(),
        m_screenAlignedQuad->program()->getUniformLocation("kernel"),
        m_occlusionOptions->kernelSize(),
        m_samplingDirections.data()
        );

    m_screenAlignedQuad->draw();
}

std::vector<float> HBAO::getSamplingDirections(int size) {
    std::vector<float> angles(size);
    std::uniform_real_distribution<float> distribution(0.0f, 2 * glm::pi<float>());

    for (auto&& angle : angles) {
        angle = distribution(m_randEngine);
    }

    return angles;
}

void HBAO::setupKernel() {
    m_samplingDirections = std::vector<float>(getSamplingDirections(m_occlusionOptions->maxKernelSize()));
}
