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
