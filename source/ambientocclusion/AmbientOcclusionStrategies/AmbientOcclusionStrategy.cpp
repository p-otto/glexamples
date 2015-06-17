#include "AmbientOcclusionStrategy.h"

#include <globjects/Program.h>
#include <globjects/Shader.h>

using namespace gl;
using namespace globjects;

AmbientOcclusionStrategy::AmbientOcclusionStrategy(const AmbientOcclusionOptions * options)
:   m_randEngine(new std::default_random_engine(std::random_device{}()))
,   m_occlusionOptions(options)
{}

globjects::Program * AmbientOcclusionStrategy::getProgram()
{
    return m_program;
}
