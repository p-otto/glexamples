#pragma once

#include <memory>
#include <vector>
#include <utility>

#include <globjects/Program.h>
#include <globjects/base/ref_ptr.h>

namespace globjects {
    class Program;
    class Texture;
    class VertexArray;
    class Buffer;
}

namespace gloperate {
    class ScreenAlignedQuad;
}

class ScreenAlignedQuadRenderer
{
public:
    ScreenAlignedQuadRenderer();
    ~ScreenAlignedQuadRenderer() = default;
    
    void setProgram(globjects::Program * program);
    void setTextures(const std::vector<std::pair<std::string, globjects::Texture*>> &stringTexturePair);
    
    globjects::Program * program();
    
    template <typename T>
    void setUniforms(const std::string &name, const T &val)
    {
        m_program->setUniform(name, val);
    }
    
    template <typename T, typename... Ts>
    void setUniforms(const std::string &name, const T &val, Ts... rest)
    {
        m_program->setUniform(name, val);
        setUniforms(rest...);
    }
    
    void draw();
private:
    globjects::ref_ptr<globjects::Program> m_program;
    globjects::ref_ptr<globjects::VertexArray> m_vao;
    globjects::ref_ptr<globjects::Buffer> m_buffer;
};
