#include "Kernel.h"
#include <glm/gtc/type_ptr.hpp>
#include <random>

namespace {
	const float DEFAULT_MIN_ANGLE = 0.1f;
	const float DEFAULT_MIN_SIZE = 0.1f;
	Kernel::LengthDistribution lDist;
	Kernel::SurfaceDistribution sDist;
	std::default_random_engine randEngine;

	float scaleFunction(float scale){
		switch (lDist) {
		case Kernel::LengthDistribution::Linear:
			return scale;
		case Kernel::LengthDistribution::Quadratic:
			return scale*scale;
		}
	}

	std::vector<glm::vec3> getSphereKernel(int size, float minSize) {
		std::vector<glm::vec3> kernel(size);
		int count = 1;

		std::uniform_real_distribution<float> distribution(-1.0, 1.0);

		for (auto &vec : kernel)
		{
			vec[0] = distribution(randEngine);
			vec[1] = distribution(randEngine);
			vec[2] = distribution(randEngine);

			vec = glm::normalize(vec);

			float scale = static_cast<float>(count++) / size;
			scale = glm::mix(minSize, 1.0f, scaleFunction(scale));
			vec *= scale;
		}

		return kernel;
	}

	std::vector<glm::vec3> getHemisphereKernel(int size, float minSize, float minAngle) {
		std::vector<glm::vec3> kernel(size);
		int count = 1;

		std::uniform_real_distribution<float> distribution(-1.0, 1.0);
		std::uniform_real_distribution<float> positive_distribution(0.0, 1.0);

		for (auto &vec : kernel)
		{
			do {
				vec[0] = distribution(randEngine);
				vec[1] = distribution(randEngine);
				vec[2] = positive_distribution(randEngine);
			} while (glm::dot(vec, glm::vec3(0, 0, 1)) < minAngle);

			vec = glm::normalize(vec);

			float scale = static_cast<float>(count++) / size;
			scale = glm::mix(minSize, 1.0f, scaleFunction(scale));
			vec *= scale;
		}

		return kernel;
	}
}

namespace Kernel {
    // TODO implement uniform surface distribution by using a icosahedron.
	

	
	std::vector<glm::vec3> getKernel(int size, float minSize, KernelType type, LengthDistribution lengthDistribution, SurfaceDistribution surfaceDistribution, float *args)
	{
		lDist = lengthDistribution;
		sDist = surfaceDistribution;
		switch (type) {
		case KernelType::Sphere:
			return getSphereKernel(size,minSize);
		case KernelType::Hemisphere:
			return getHemisphereKernel(size, minSize, (args != nullptr) ? args[0] : DEFAULT_MIN_ANGLE);
		}
	}

	std::vector<glm::vec3> getKernel(int size, KernelType type, LengthDistribution lengthDistribution, SurfaceDistribution surfaceDistribution, float *args){
		return getKernel(size, DEFAULT_MIN_SIZE, type, lengthDistribution, surfaceDistribution, args);
	}

	std::vector<glm::vec3> getKernel(int size, KernelType type, LengthDistribution lengthDistribution, SurfaceDistribution surfaceDistribution){
		return getKernel(size, DEFAULT_MIN_SIZE, type, lengthDistribution, surfaceDistribution,nullptr);
	}
	std::vector<glm::vec3> getKernel(int size, int minSize,KernelType type, LengthDistribution lengthDistribution, SurfaceDistribution surfaceDistribution){
		return getKernel(size, minSize, type, lengthDistribution, surfaceDistribution, nullptr);
	}
}
