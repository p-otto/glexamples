#include "AmbientOcclusionStage.h"

#include "ScreenAlignedQuadRenderer.h"

#include <glbinding/gl/enum.h>
#include <glbinding/gl/bitfield.h>
#include <glbinding/gl/functions.h>

#include <globjects/Texture.h>
#include <globjects/Framebuffer.h>
#include <globjects/Shader.h>

#include <gloperate/primitives/UniformGroup.h>
#include <gloperate/base/make_unique.hpp>

#include <glm/gtc/type_ptr.hpp>

using namespace gl;
using namespace globjects;

AmbientOcclusionStage::AmbientOcclusionStage(const AmbientOcclusionOptions * options)
:   m_occlusionOptions(options)
,   m_uniformGroup(gloperate::make_unique<gloperate::UniformGroup>())
{}

void AmbientOcclusionStage::initialize()
{
    m_screenAlignedQuad = gloperate::make_unique<ScreenAlignedQuadRenderer>();

    m_occlusionAttachment = Texture::createDefault(GL_TEXTURE_2D);

    m_occlusionFbo = make_ref<Framebuffer>();
    m_occlusionFbo->attachTexture(GL_COLOR_ATTACHMENT0, m_occlusionAttachment);

    initializeMethodSpecific();
}

void AmbientOcclusionStage::initializeMethodSpecific()
{}

void AmbientOcclusionStage::updateFramebuffer(const int width, const int height)
{
    auto occlusionWidth = width, occlusionHeight = height;

    if (m_occlusionOptions->halfResolution())
    {
        occlusionHeight /= 2;
        occlusionWidth /= 2;
    }

    m_occlusionAttachment->image2D(0, GL_RGB8, occlusionWidth, occlusionHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    updateFramebufferMethodSpecific(occlusionWidth, occlusionHeight);
}

void AmbientOcclusionStage::updateFramebufferMethodSpecific(const int, const int)
{}

void AmbientOcclusionStage::process(globjects::Texture * normalsDepth, std::vector<globjects::Texture*>)
{
    m_screenAlignedQuad->setProgram(m_program);

    m_occlusionFbo->bind();
    m_occlusionFbo->clearBuffer(GL_COLOR, 0, glm::vec4{0.0, 0.0, 0.0, 0.0});

    m_screenAlignedQuad->setTextures({
        { "u_normal_depth", normalsDepth },
        { "u_rotation", m_rotationTex }
    });

    m_uniformGroup->addToProgram(m_screenAlignedQuad->program());

    // don't upload kernel for SSAONone
    if (m_kernel.size() > 0)
    {
        glProgramUniform3fv(
            m_screenAlignedQuad->program()->id(),
            m_screenAlignedQuad->program()->getUniformLocation("kernel"),
            m_occlusionOptions->kernelSize(),
            glm::value_ptr((m_kernel)[0])
        );
    }

    m_screenAlignedQuad->draw();
}

globjects::Texture* AmbientOcclusionStage::getOcclusionTexture()
{
    return m_occlusionAttachment;
}

gloperate::UniformGroup* AmbientOcclusionStage::getUniformGroup()
{
    return m_uniformGroup.get();
}

std::vector<glm::vec3> AmbientOcclusionStage::getKernel(int size)
{
    return Kernel::getKernel(size, m_occlusionOptions->minimalKernelLength(), m_occlusionOptions->minimalKernelAngle(), getKernelType(), m_occlusionOptions->lengthDistribution(), m_occlusionOptions->surfaceDistribution());
}

void AmbientOcclusionStage::setupKernel()
{
    m_kernel = std::vector<glm::vec3>(getKernel(m_occlusionOptions->maxKernelSize()));
}

void AmbientOcclusionStage::setupRotationTex()
{
    std::vector<glm::vec3> rotationValues = getNoiseTexture(m_occlusionOptions->rotationTexSize());

    if (!m_rotationTex)
    {
        m_rotationTex = Texture::createDefault(GL_TEXTURE_2D);
        m_rotationTex->setParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
        m_rotationTex->setParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
        m_rotationTex->setParameter(GL_TEXTURE_WRAP_R, GL_REPEAT);
    }

    m_rotationTex->image2D(0, GL_RGB32F, m_occlusionOptions->rotationTexSize(), m_occlusionOptions->rotationTexSize(), 0, GL_RGB, GL_FLOAT, rotationValues.data());
}
