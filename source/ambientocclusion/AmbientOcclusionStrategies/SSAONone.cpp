#include "SSAONone.h"

#include "AmbientOcclusionOptions.h"

#include <glbinding/gl/enum.h>

#include <globjects/Program.h>
#include <globjects/Shader.h>

using namespace gl;
using namespace globjects;

SSAONone::SSAONone(const AmbientOcclusionOptions * options)
:   AmbientOcclusionStrategy(options)
{
    m_program = new Program{};
    m_program->attach(
        Shader::fromFile(GL_VERTEX_SHADER, "data/ambientocclusion/screen_quad.vert"),
        Shader::fromFile(GL_FRAGMENT_SHADER, "data/ambientocclusion/ssao_none.frag")
    );
}

std::vector<glm::vec3> SSAONone::getKernel(int size)
{
    return {};
}

std::vector<glm::vec3> SSAONone::getNoiseTexture(int size)
{
    return {};
}
