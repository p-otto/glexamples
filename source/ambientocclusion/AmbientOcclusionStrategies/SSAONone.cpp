#include "SSAONone.h"

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

SSAONone::SSAONone(const AmbientOcclusionOptions * options)
:   AmbientOcclusionStage(options)
{
    m_program = new Program{};
    m_program->attach(
        Shader::fromFile(GL_VERTEX_SHADER, "data/ambientocclusion/screen_quad.vert"),
        Shader::fromFile(GL_FRAGMENT_SHADER, "data/ambientocclusion/ssao_none.frag")
    );
}

Kernel::KernelType SSAONone::getKernelType()
{
    return Kernel::KernelType::Hemisphere;
}

std::vector<glm::vec3> SSAONone::getNoiseTexture(int)
{
    return {};
}
