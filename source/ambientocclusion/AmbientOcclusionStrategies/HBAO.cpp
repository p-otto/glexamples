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

Kernel::KernelType HBAO::getKernelType()
{
    return Kernel::KernelType::Hemisphere;
}

std::vector<glm::vec3> HBAO::getNoiseTexture(int size) {
    std::vector<glm::vec3> tex(size * size);

	std::uniform_real_distribution<float> angleDistribution(0.0f, 2.0f * glm::pi<float>());
    std::uniform_real_distribution<float> distribution(-0.5f, 0.5f);

	for (auto &vec : tex) {
		auto angle = angleDistribution(m_randEngine);
		vec[0] = glm::sin(angle);
		vec[1] = glm::cos(angle);
        vec[2] = distribution(m_randEngine);
    }

    return tex;
}

