#include "HBAO.h"

#include "AmbientOcclusionOptions.h"

#include <glbinding/gl/enum.h>

#include <globjects/Program.h>
#include <globjects/Shader.h>

using namespace gl;
using namespace globjects;

HBAO::HBAO(const AmbientOcclusionOptions * options)
:   AmbientOcclusionStrategy(options)
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
