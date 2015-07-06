#include "SSAOHemisphere.h"

#include "ScreenAlignedQuadRenderer.h"

#include <Kernel.h>

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

SSAOHemisphere::SSAOHemisphere(const AmbientOcclusionOptions * options)
:   AmbientOcclusionStage(options)
{
    m_program = new Program{};
    m_program->attach(
        Shader::fromFile(GL_VERTEX_SHADER, "data/ambientocclusion/screen_quad.vert"),
        Shader::fromFile(GL_FRAGMENT_SHADER, "data/ambientocclusion/ssao_normaloriented.frag")   
    );
}

std::vector<glm::vec3> SSAOHemisphere::getKernel(int size)
{


	// keine ahnung wie man inline ein float array erstellt. 
	// TODO: haesslichen workaround fixen
	// wuerde sowas gehen wie &m_occlus... ? ist ja ein pointer theoretisch
	float howdoesthisevenwork[] = { m_occlusionOptions->minimalKernelAngle() };

	// erstelle Kernel mit default Hemisphere optionen
	return Kernel::getKernel(size, Kernel::KernelType::Hemisphere, Kernel::LengthDistribution::Quadratic, Kernel::SurfaceDistribution::Random, howdoesthisevenwork);
}

std::vector<glm::vec3> SSAOHemisphere::getNoiseTexture(int size)
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
