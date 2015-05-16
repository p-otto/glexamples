#include "ScreenAlignedQuadRenderer.h"

#include <globjects/VertexArray.h>
#include <globjects/VertexAttributeBinding.h>
#include <globjects/Buffer.h>

#include <gloperate/primitives/ScreenAlignedQuad.h>

#include <glbinding/gl/enum.h>

ScreenAlignedQuadRenderer::ScreenAlignedQuadRenderer()
{
    static const std::array<glm::vec2, 4> raw{{
        glm::vec2( +1.f, -1.f )
        ,   glm::vec2( +1.f, +1.f )
        ,   glm::vec2( -1.f, -1.f )
        ,   glm::vec2( -1.f, +1.f )
    }};
    
    m_vao = new globjects::VertexArray();
    m_buffer = new globjects::Buffer();
    m_buffer->setData(raw, gl::GL_STATIC_DRAW);
    
    auto binding = m_vao->binding(0);
    binding->setAttribute(0);
    binding->setBuffer(m_buffer, 0, sizeof(glm::vec2));
    binding->setFormat(2, gl::GL_FLOAT, gl::GL_FALSE, 0);
    m_vao->enable(0);
}

void ScreenAlignedQuadRenderer::setProgram(globjects::Program * program)
{
    m_program = program;
}

globjects::Program * ScreenAlignedQuadRenderer::program()
{
    return m_program;
}

void ScreenAlignedQuadRenderer::draw()
{
    m_program->use();
    m_vao->drawArrays(gl::GL_TRIANGLE_STRIP, 0, 4);
    m_program->release();
}

void ScreenAlignedQuadRenderer::setTextures(const std::vector<std::pair<std::string, globjects::Texture*>> &stringTexturePair)
{
    for (int i = 0; i < stringTexturePair.size(); ++i)
    {
        auto &pair = stringTexturePair.at(i);
        pair.second->bindActive(gl::GL_TEXTURE0 + i);
        m_program->setUniform(pair.first, i);
    }
}
