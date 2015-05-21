#include "Plane.h"

#include <vector>

#include <glbinding/gl/functions.h>
#include <glbinding/gl/enum.h>

#include <globjects/Buffer.h>
#include <globjects/VertexArray.h>
#include <globjects/VertexAttributeBinding.h>

#include <glm/glm.hpp>

Plane::Plane()
{
    static const float length = 20.0f;
    
    std::vector<glm::vec3> points{
        glm::vec3(-length, 0, -length),
        glm::vec3(-length, 0, length),
        glm::vec3(length, 0, -length),
        glm::vec3(length, 0, length)
    };
    
    std::vector<glm::vec3> normals{
        glm::vec3(0, 1, 0),
        glm::vec3(0, 1, 0),
        glm::vec3(0, 1, 0),
        glm::vec3(0, 1, 0)
    };
    
    m_positionBuffer = new globjects::Buffer;
    m_positionBuffer->setData(points, gl::GL_STATIC_DRAW);
    
    m_normalBuffer = new globjects::Buffer;
    m_normalBuffer->setData(normals, gl::GL_STATIC_DRAW);
    
    m_vao = new globjects::VertexArray;
    auto binding = m_vao->binding(0);
    
    binding->setAttribute(0);
    binding->setBuffer(m_positionBuffer, 0, sizeof(glm::vec3));
    binding->setFormat(3, gl::GL_FLOAT, gl::GL_FALSE, 0);
    
    binding = m_vao->binding(1);
    binding->setAttribute(1);
    binding->setBuffer(m_normalBuffer, 0, sizeof(glm::vec3));
    binding->setFormat(3, gl::GL_FLOAT, gl::GL_FALSE, 0);
    
    m_vao->enable(0);
    m_vao->enable(1);
}

void Plane::draw()
{
    m_vao->bind();
    m_vao->drawArrays(gl::GL_TRIANGLE_STRIP, 0, 4);
    m_vao->unbind();
}
