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

std::vector<glm::vec3> HBAO::getNoiseTexture(int size) {
    std::vector<glm::vec3> tex(size * size);

    // TODO: make actual distribution for normals
    std::uniform_real_distribution<float> circleDistribution(-1.0, 1.0);
    std::uniform_real_distribution<float> distribution(0.0, 1.0);

    for (auto &vec : tex) {
        vec[0] = circleDistribution(m_randEngine);
        vec[1] = circleDistribution(m_randEngine);
        vec[2] = 0.0f;

        vec = glm::normalize(vec);
        vec[2] = distribution(m_randEngine);
    }

    return tex;
}

