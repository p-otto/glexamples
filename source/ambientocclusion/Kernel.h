#pragma once

#include <vector>
#include <glm/glm.hpp>

namespace AmbientOcclusionKernel {
    enum Type { SphereKernel, HemisphereKernel };

    std::vector<glm::vec3> uniformKernel(Type type);
    std::vector<glm::vec3> quadraticKernel(Type type);
    std::vector<glm::vec3> starcraftKernel(Type type);
}
