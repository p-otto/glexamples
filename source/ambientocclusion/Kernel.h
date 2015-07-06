#pragma once

#include <vector>
#include <glm/glm.hpp>

namespace Kernel {
	enum class KernelType { Sphere, Hemisphere };
	enum class LengthDistribution { Linear, Quadratic, Starcraft };
	enum class SurfaceDistribution { Random, Uniform };

	std::vector<glm::vec3> getKernel(int size, float minSize, KernelType type, LengthDistribution lengthDistribution, SurfaceDistribution surfaceDistribution, float *args);
	std::vector<glm::vec3> getKernel(int size, KernelType type, LengthDistribution lengthDistribution, SurfaceDistribution surfaceDistribution,float *args);
	std::vector<glm::vec3> getKernel(int size, KernelType type, LengthDistribution lengthDistribution, SurfaceDistribution surfaceDistribution);
	std::vector<glm::vec3> getKernel(int size, int minSize, KernelType type, LengthDistribution lengthDistribution, SurfaceDistribution surfaceDistribution);
}