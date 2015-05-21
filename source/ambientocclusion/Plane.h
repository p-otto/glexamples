#pragma once

#include <globjects/base/ref_ptr.h>

namespace globjects {
    class VertexArray;
    class Buffer;
}

class Plane
{
public:
    Plane();
    
    void draw();
    
private:
    globjects::ref_ptr<globjects::VertexArray> m_vao;
    globjects::ref_ptr<globjects::Buffer> m_positionBuffer;
    globjects::ref_ptr<globjects::Buffer> m_normalBuffer;
};
