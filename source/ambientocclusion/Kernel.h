#pragma once

#include <vector>
#include <glm/glm.hpp>

namespace Kernel {
	enum class KernelType { Sphere, Hemisphere };
	enum class LengthDistribution { Linear, Quadratic, Starcraft };
	enum class SurfaceDistribution { Random, Uniform };

	std::vector<glm::vec3> getKernel(int size, float minSize, float minAngle, KernelType type, LengthDistribution lengthDistribution, SurfaceDistribution surfaceDistribution);
}