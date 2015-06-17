#pragma once

#include "AmbientOcclusionOptions.h"

#include <random>
#include <memory>

#include <glm/glm.hpp>

#include <globjects/base/ref_ptr.h>

namespace globjects {
    class Program;
}

class AmbientOcclusionStrategy
{
public:
    AmbientOcclusionStrategy(const AmbientOcclusionOptions * options);
    virtual ~AmbientOcclusionStrategy() = default;

    globjects::Program * getProgram();

    virtual std::vector<glm::vec3> getKernel(int size) = 0;
    virtual std::vector<glm::vec3> getNoiseTexture(int size) = 0;

protected:
    const AmbientOcclusionOptions * m_occlusionOptions;

    std::unique_ptr<std::default_random_engine> m_randEngine;
    globjects::ref_ptr<globjects::Program> m_program;
};
