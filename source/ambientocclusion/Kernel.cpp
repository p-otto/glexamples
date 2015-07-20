#include "Kernel.h"
#include <glm/gtc/type_ptr.hpp>
#include <random>

namespace
{
	Kernel::LengthDistribution lDist;
	Kernel::SurfaceDistribution sDist;
	std::default_random_engine randEngine;

	float scaleFunction(float scale, int sample, int maxSample){
		switch (lDist) {
		case Kernel::LengthDistribution::Linear:
			return scale;
		case Kernel::LengthDistribution::Quadratic:
			return scale*scale;
        case Kernel::LengthDistribution::Starcraft:
            // one fourth of the samples is nearer to the center
            if (sample > 3 * maxSample / 4) {
                scale *= 4;
            }
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

			float scale = static_cast<float>(count) / size;
			scale = glm::mix(minSize, 1.0f, scaleFunction(scale, count, size));
			vec *= scale;
            ++count;
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

			float scale = static_cast<float>(count) / size;
			scale = glm::mix(minSize, 1.0f, scaleFunction(scale, count, size));
			vec *= scale;
            ++count;
		}

		return kernel;
	}
}

namespace Kernel
{
    // TODO: implement uniform surface distribution by using a icosahedron.
	std::vector<glm::vec3> getKernel(int size, float minSize, float minAngle, KernelType type, LengthDistribution lengthDistribution, SurfaceDistribution surfaceDistribution)
	{
		lDist = lengthDistribution;
		sDist = surfaceDistribution;
		switch (type) {
		case KernelType::Sphere:
			return getSphereKernel(size, minSize);
		case KernelType::Hemisphere:
			return getHemisphereKernel(size, minSize, minAngle);
		}
	}
}
