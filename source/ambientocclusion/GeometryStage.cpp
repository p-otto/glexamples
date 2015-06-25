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
    m_modelProgram = new Program{};
    m_modelProgram->attach(
        Shader::fromFile(GL_VERTEX_SHADER, "data/ambientocclusion/model.vert"),
        Shader::fromFile(GL_FRAGMENT_SHADER, "data/ambientocclusion/model.frag")
    );

    m_phongProgram = new Program{};
    m_phongProgram->attach(
        Shader::fromFile(GL_VERTEX_SHADER, "data/ambientocclusion/model.vert"),
        Shader::fromFile(GL_FRAGMENT_SHADER, "data/ambientocclusion/phong.frag")
    );

    // Model
    for (auto & mesh : scene->meshes())
    {
        m_drawables.push_back(gloperate::PolygonalDrawable(*mesh));
    }

    m_plane = gloperate::make_unique<Plane>();

    // Framebuffer
    m_colorAttachment = Texture::createDefault(GL_TEXTURE_2D);
    m_normalDepthAttachment = Texture::createDefault(GL_TEXTURE_2D);
    m_depthBuffer = Texture::createDefault(GL_TEXTURE_2D);

    m_modelFbo = make_ref<Framebuffer>();
    m_modelFbo->attachTexture(GL_COLOR_ATTACHMENT0, m_colorAttachment);
    m_modelFbo->attachTexture(GL_COLOR_ATTACHMENT1, m_normalDepthAttachment);
    m_modelFbo->attachTexture(GL_DEPTH_ATTACHMENT, m_depthBuffer);
    m_modelFbo->setDrawBuffers({GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1});
}

void GeometryStage::updateFramebuffer(const int width, const int height)
{
    m_colorAttachment->image2D(0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    m_normalDepthAttachment->image2D(0, GL_RGBA16, width, height, 0, GL_RGBA, GL_UNSIGNED_SHORT, nullptr);
    m_depthBuffer->image2D(0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);
}

void GeometryStage::process()
{
    glm::ivec3 color{21, 243, 103};
    auto program = m_occlusionOptions->phong() ? m_phongProgram : m_modelProgram;

    program->use();

    m_uniformGroup->addToProgram(program);

    m_plane->draw();
    for (auto & drawable : m_drawables)
    {
        program->setUniform("u_color", glm::vec3(color) / 256.0f);
        drawable.draw();

        color = glm::vec3((color.r + 68) % 256,
                          (color.g + 64) % 256,
                          (color.b + 99) % 256
        );
    }

    program->release();
}

void GeometryStage::bindAndClearFbo()
{
    m_modelFbo->bind(GL_FRAMEBUFFER);
    m_modelFbo->clearBuffer(GL_COLOR, 0, glm::vec4{0.85f, 0.87f, 0.91f, 1.0f});
    m_modelFbo->clearBuffer(GL_COLOR, 1, glm::vec4{0.0f, 0.0f, 0.0f, 1.0f});
    m_modelFbo->clearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0);
}

globjects::Texture * GeometryStage::getColorTexture()
{
    return m_colorAttachment;
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
