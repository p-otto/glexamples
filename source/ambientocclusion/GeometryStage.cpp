#include "GeometryStage.h"

#include "AmbientOcclusionOptions.h"
#include "Plane.h"

#include <glbinding/gl/enum.h>
#include <glbinding/gl/bitfield.h>
#include <glbinding/gl/functions.h>
#include <glbinding/gl/boolean.h>

#include <globjects/Texture.h>
#include <globjects/Framebuffer.h>
#include <globjects/Program.h>
#include <globjects/Shader.h>

#include <gloperate/primitives/Scene.h>
#include <gloperate/primitives/PolygonalDrawable.h>
#include <gloperate/primitives/UniformGroup.h>

#include <glm/glm.hpp>

#include <gloperate/base/make_unique.hpp>

using namespace gl;
using namespace globjects;

GeometryStage::GeometryStage(const AmbientOcclusionOptions * options)
:   m_occlusionOptions(options)
,   m_uniformGroup(gloperate::make_unique<gloperate::UniformGroup>())
{}

void GeometryStage::initialize(const gloperate::Scene * scene)
{
    // Shader
    m_phongProgram = new Program{};
    m_phongProgram->attach(
        Shader::fromFile(GL_VERTEX_SHADER, "data/ambientocclusion/model.vert"),
        Shader::fromFile(GL_FRAGMENT_SHADER, "data/ambientocclusion/phong.frag", { "/lights" })
    );

    // Model
    for (auto & mesh : scene->meshes())
    {
        m_drawables.push_back(gloperate::PolygonalDrawable(*mesh));
    }

    m_plane = gloperate::make_unique<Plane>();

    // Framebuffer
    m_ambientAttachment = Texture::createDefault(GL_TEXTURE_2D);
    m_diffuseAttachment = Texture::createDefault(GL_TEXTURE_2D);
    m_normalDepthAttachment = Texture::createDefault(GL_TEXTURE_2D);
    m_depthBuffer = Texture::createDefault(GL_TEXTURE_2D);

    m_modelFbo = make_ref<Framebuffer>();
    m_modelFbo->attachTexture(GL_COLOR_ATTACHMENT0, m_ambientAttachment);
    m_modelFbo->attachTexture(GL_COLOR_ATTACHMENT1, m_diffuseAttachment);
    m_modelFbo->attachTexture(GL_COLOR_ATTACHMENT2, m_normalDepthAttachment);
    m_modelFbo->attachTexture(GL_DEPTH_ATTACHMENT, m_depthBuffer);
    m_modelFbo->setDrawBuffers({GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2});
}

void GeometryStage::updateFramebuffer(const int width, const int height)
{
    m_ambientAttachment->image2D(0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    m_diffuseAttachment->image2D(0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    m_normalDepthAttachment->image2D(0, GL_RGBA16, width, height, 0, GL_RGBA, GL_UNSIGNED_SHORT, nullptr);
    m_depthBuffer->image2D(0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);
}

void GeometryStage::process()
{
    m_modelFbo->bind(GL_FRAMEBUFFER);
    m_modelFbo->clearBuffer(GL_COLOR, 0, glm::vec4{ 0.85f, 0.87f, 0.91f, 1.0f });
    m_modelFbo->clearBuffer(GL_COLOR, 1, glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f });
    m_modelFbo->clearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0);

    glm::ivec3 color{21, 243, 103};

    m_phongProgram->use();

    m_uniformGroup->addToProgram(m_phongProgram);

    m_plane->draw();
    for (auto & drawable : m_drawables)
    {
        if (m_occlusionOptions->color()) {
            m_phongProgram->setUniform("u_color", glm::vec3(color) / 256.0f);
        }
        else {
            m_phongProgram->setUniform("u_color", glm::vec3(1.0));
        }
        drawable.draw();

        color = glm::vec3((color.r + 68) % 256,
                          (color.g + 64) % 256,
                          (color.b + 99) % 256
        );
    }

    m_phongProgram->release();
}

globjects::Texture * GeometryStage::getAmbientTexture()
{
    return m_ambientAttachment;
}

globjects::Texture * GeometryStage::getDiffuseTexture()
{
    return m_diffuseAttachment;
}

globjects::Texture * GeometryStage::getNormalDepthTexture()
{
    return m_normalDepthAttachment;
}

globjects::Texture * GeometryStage::getDepthBuffer()
{
    return m_depthBuffer;
}

gloperate::UniformGroup * GeometryStage::getUniformGroup()
{
    return m_uniformGroup.get();
}
