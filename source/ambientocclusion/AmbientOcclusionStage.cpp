#include "AmbientOcclusionStage.h"

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

    m_noSSAO = gloperate::make_unique<SSAONone>(m_occlusionOptions);
    m_sphereSSAO = gloperate::make_unique<SSAOSphere>(m_occlusionOptions);
    m_hemisphereSSAO = gloperate::make_unique<SSAOHemisphere>(m_occlusionOptions);
    
    setAmbientOcclusion(m_occlusionOptions->ambientOcclusion());
}

void AmbientOcclusionStage::setAmbientOcclusion(const AmbientOcclusionType &type)
{
    switch (type)
    {
    case ScreenSpaceSphere:
        m_strategy = m_sphereSSAO.get();
        break;
    case ScreenSpaceHemisphere:
        m_strategy = m_hemisphereSSAO.get();
        break;
    default:
        m_strategy = m_noSSAO.get();
        break;
    }
    setupKernelAndRotationTex();
}

void AmbientOcclusionStage::updateFramebuffer(const int width, const int height)
{
    auto occlusionWidth = width, occlusionHeight = height;

    if (m_occlusionOptions->halfResolution())
    {
        occlusionHeight /= 2;
        occlusionWidth /= 2;
    }

    m_occlusionAttachment->image2D(0, GL_R8, occlusionWidth, occlusionHeight, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    m_occlusionAttachment->image2D(0, GL_R8, occlusionWidth, occlusionHeight, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
}

void AmbientOcclusionStage::process(globjects::Texture *normalsDepth)
{
    m_screenAlignedQuad->setProgram(m_strategy->getProgram());

    m_occlusionFbo->bind();
    m_occlusionFbo->clearBuffer(GL_COLOR, 0, glm::vec4{0.0, 0.0, 0.0, 0.0});

    m_screenAlignedQuad->setTextures({
        { "u_normal_depth", normalsDepth },
        { "u_rotation", m_rotationTex }
    });

    m_uniformGroup->addToProgram(m_screenAlignedQuad->program());

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

gloperate::UniformGroup* AmbientOcclusionStage::getUniformGroup()
{
    return m_uniformGroup.get();
}

void AmbientOcclusionStage::setupKernelAndRotationTex()
{
    m_kernel = std::vector<glm::vec3>(m_strategy->getKernel(m_occlusionOptions->maxKernelSize()));
    std::vector<glm::vec3> rotationValues = m_strategy->getNoiseTexture(m_occlusionOptions->rotationTexSize());

    if (!m_rotationTex)
    {
        m_rotationTex = Texture::createDefault(GL_TEXTURE_2D);
        m_rotationTex->setParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
        m_rotationTex->setParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
        m_rotationTex->setParameter(GL_TEXTURE_WRAP_R, GL_REPEAT);
    }

    m_rotationTex->image2D(0, GL_RGB32F, m_occlusionOptions->rotationTexSize(), m_occlusionOptions->rotationTexSize(), 0, GL_RGB, GL_FLOAT, rotationValues.data());
}
