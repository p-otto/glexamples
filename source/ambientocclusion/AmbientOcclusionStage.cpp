#include "AmbientOcclusionStage.h"

#include "AmbientOcclusionOptions.h"
#include "ScreenAlignedQuadRenderer.h"

#include <glbinding/gl/enum.h>
#include <glbinding/gl/bitfield.h>
#include <glbinding/gl/functions.h>

#include <globjects/Texture.h>
#include <globjects/Framebuffer.h>
#include <globjects/Program.h>
#include <globjects/Shader.h>

#include <gloperate/base/make_unique.hpp>

#include <glm/gtc/type_ptr.hpp>

using namespace gl;
using namespace globjects;

AmbientOcclusionStage::AmbientOcclusionStage(const AmbientOcclusionOptions *options)
:   m_randEngine(new std::default_random_engine(std::random_device{}()))
{}

void AmbientOcclusionStage::initialize()
{
    m_screenAlignedQuad = gloperate::make_unique<ScreenAlignedQuadRenderer>();

    m_occlusionAttachment = Texture::createDefault(GL_TEXTURE_2D);

    m_occlusionFbo = make_ref<Framebuffer>();
    m_occlusionFbo->attachTexture(GL_COLOR_ATTACHMENT0, m_occlusionAttachment);

    m_occlusionFbo->printStatus(true);

    m_ambientOcclusionProgramNormalOriented = new Program{};
    m_ambientOcclusionProgramNormalOriented->attach(
        Shader::fromFile(GL_VERTEX_SHADER, "data/ambientocclusion/screen_quad.vert"),
        Shader::fromFile(GL_FRAGMENT_SHADER, "data/ambientocclusion/ssao_normaloriented.frag")
    );

    m_ambientOcclusionProgramCrytek = new Program{};
    m_ambientOcclusionProgramCrytek->attach(
        Shader::fromFile(GL_VERTEX_SHADER, "data/ambientocclusion/screen_quad.vert"),
        Shader::fromFile(GL_FRAGMENT_SHADER, "data/ambientocclusion/ssao_crytek.frag")
    );

}

void AmbientOcclusionStage::updateFramebuffer(const int width, const int height)
{
    auto occlusionWidth = width, occlusionHeight = height;

    if (m_occlusionOptions->halfResolution()) {
        occlusionHeight /= 2;
        occlusionWidth /= 2;
    }

    m_occlusionAttachment->image2D(0, GL_R8, occlusionWidth, occlusionHeight, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    m_occlusionAttachment->image2D(0, GL_R8, occlusionWidth, occlusionHeight, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
}

void AmbientOcclusionStage::process(globjects::Texture *normalsDepth, )
{
    if (m_occlusionOptions->normalOriented())
    {
        m_screenAlignedQuad->setProgram(m_ambientOcclusionProgramNormalOriented);
    }
    else
    {
        m_screenAlignedQuad->setProgram(m_ambientOcclusionProgramCrytek);
    }

    m_occlusionFbo->bind();
    m_occlusionFbo->clearBuffer(GL_COLOR, 0, glm::vec4{0.0, 0.0, 0.0, 0.0});

    m_screenAlignedQuad->setTextures({
        { "u_normal_depth", normalsDepth },
        { "u_rotation", m_rotationTex }
    });

    m_screenAlignedQuad->setUniforms(
                                     "u_invProj", glm::inverse(m_projectionCapability->projection()),
                                     "u_proj", m_projectionCapability->projection(),
                                     "u_farPlane", m_projectionCapability->zFar(),
                                     "u_resolutionX", m_viewportCapability->width(),
                                     "u_resolutionY", m_viewportCapability->height(),
                                     "u_kernelSize", m_occlusionOptions->kernelSize(),
                                     "u_kernelRadius", m_occlusionOptions->kernelRadius(),
                                     "u_attenuation", m_occlusionOptions->attenuation()
                                     );
    
    glProgramUniform3fv(
        m_screenAlignedQuad->program()->id(),
        m_screenAlignedQuad->program()->getUniformLocation("kernel"),
        m_occlusionOptions->kernelSize(),
        glm::value_ptr((m_kernel)[0])
    );
    
    m_screenAlignedQuad->draw();
}

globjects::Texture* AmbientOcclusionStage::getOcclusionTexture()
{
    return m_occlusionAttachment;
}

void AmbientOcclusionStage::setupKernelAndRotationTex()
{
    std::vector<glm::vec3> rotationValues;
    if (m_occlusionOptions->normalOriented()) {
        m_kernel = std::vector<glm::vec3>(getHemisphereKernel(m_occlusionOptions->maxKernelSize()));
        rotationValues = getRotationTexture(m_occlusionOptions->rotationTexSize());
    }
    else {
        m_kernel = std::vector<glm::vec3>(getSphereKernel(m_occlusionOptions->maxKernelSize()));
        rotationValues = getReflectionTexture(m_occlusionOptions->rotationTexSize());
    }

    if (!m_rotationTex)
    {
        m_rotationTex = Texture::createDefault(GL_TEXTURE_2D);
        m_rotationTex->setParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
        m_rotationTex->setParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
        m_rotationTex->setParameter(GL_TEXTURE_WRAP_R, GL_REPEAT);
    }

    m_rotationTex->image2D(0, GL_RGB32F, m_occlusionOptions->rotationTexSize(), m_occlusionOptions->rotationTexSize(), 0, GL_RGB, GL_FLOAT, rotationValues.data());
}

std::vector<glm::vec3> AmbientOcclusionStage::getHemisphereKernel(int size)
{
    std::vector<glm::vec3> kernel(size);
    int count = 1;

    std::uniform_real_distribution<float> distribution(-1.0, 1.0);
    std::uniform_real_distribution<float> positive_distribution(0.0, 1.0);

    for (auto &vec : kernel)
    {
        do {
            vec[0] = distribution(*m_randEngine);
            vec[1] = distribution(*m_randEngine);
            vec[2] = positive_distribution(*m_randEngine);
        } while(glm::dot(vec, glm::vec3(0,0,1)) < m_occlusionOptions->minimalKernelAngle());

        vec = glm::normalize(vec);

        float scale = static_cast<float>(count++) / size;
        scale = glm::mix(m_occlusionOptions->minimalKernelLength(), 1.0f, scale * scale);
        vec *= scale;
    }

    return kernel;
}

std::vector<glm::vec3> AmbientOcclusionStage::getSphereKernel(int size)
{
    std::vector<glm::vec3> kernel(size);
    int count = 1;

    std::uniform_real_distribution<float> distribution(-1.0, 1.0);

    for (auto &vec : kernel)
    {
        vec[0] = distribution(*m_randEngine);
        vec[1] = distribution(*m_randEngine);
        vec[2] = distribution(*m_randEngine);

        vec = glm::normalize(vec);

        float scale = static_cast<float>(count++) / size;
        scale = glm::mix(m_occlusionOptions->minimalKernelLength(), 1.0f, scale * scale);
        vec *= scale;
    }

    return kernel;
}

std::vector<glm::vec3> AmbientOcclusionStage::getRotationTexture(int size)
{
    std::vector<glm::vec3> tex(size * size);

    std::uniform_real_distribution<float> distribution(-1.0, 1.0);

    for (auto &vec : tex)
    {
        vec[0] = distribution(*m_randEngine);
        vec[1] = distribution(*m_randEngine);
        vec[2] = 0.0f;

        vec = glm::normalize(vec);
    }

    return tex;
}

std::vector<glm::vec3> AmbientOcclusionStage::getReflectionTexture(int size)
{
    std::vector<glm::vec3> tex(size * size);

    std::uniform_real_distribution<float> distribution(-1.0, 1.0);

    for (auto &vec : tex)
    {
        vec[0] = distribution(*m_randEngine);
        vec[1] = distribution(*m_randEngine);
        vec[2] = distribution(*m_randEngine);

        vec = glm::normalize(vec);
    }

    return tex;
}