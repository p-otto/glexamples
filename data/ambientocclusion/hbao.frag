#version 150 core
#extension GL_ARB_explicit_attrib_location : require

in vec2 v_uv;
in vec3 v_viewRay;

uniform sampler2D u_normal_depth;
#define ROTATION_SIZE 4
uniform sampler2D u_rotation;

uniform int u_kernelSize;
uniform float u_kernelRadius;
uniform mat4 u_proj;
uniform int u_resolutionX;
uniform int u_resolutionY;
#define NUM_SAMPLES 5

#define MAX_KERNEL_SIZE 128
uniform float kernel[MAX_KERNEL_SIZE];

layout(location = 0) out vec3 occlusion;

const float pi = 3.14159265;

vec3 calcPosition(float depth)
{
    return v_viewRay * depth;
}

vec2 snapToGrid(vec2 offset)
{
	return round(offset * vec2(u_resolutionX, u_resolutionY)) / vec2(u_resolutionX, u_resolutionY);
}

float findHorizonAngle(vec2 offset, int numSamples, float depth)
{
	float largestHorizonAngle = 0.0;
	for (int step = 1; step < numSamples; step++) {
		vec2 sampleOffset = offset * step;
		float sampleDepth = texture(u_normal_depth, v_uv + sampleOffset).a;
		float horizonAngle = atan((sampleDepth - depth)/ length(sampleOffset));

		largestHorizonAngle = max(largestHorizonAngle, horizonAngle);
	}
	return largestHorizonAngle;
}

vec2 rotate(vec2 direction, vec2 rand) {
	return vec2(direction.x*rand.x - direction.y*rand.y,
                  direction.x*rand.y + direction.y*rand.x);
}

void main()
{
    float depth = texture(u_normal_depth, v_uv).a;
    vec3 normal = texture(u_normal_depth, v_uv).rgb * 2.0 - vec3(1.0);
    normal = normalize(normal);

    vec3 position = calcPosition(depth);

    float ambientOcclusion = 0.0;
    float stepSize = u_kernelRadius / NUM_SAMPLES;

    vec2 noise_scale = vec2(u_resolutionX / ROTATION_SIZE, u_resolutionY / ROTATION_SIZE);
    vec2 rvec = texture(u_rotation, v_uv * noise_scale).xy;

    for (int i = 0; i < u_kernelSize; i++) {
    	vec2 sampleDirection = rotate(vec2(sin(kernel[i]), cos(kernel[i])), rvec);
    	sampleDirection = normalize(sampleDirection);
    	vec4 scaledDirection = u_proj * vec4(sampleDirection * stepSize, 0.0, 0.0);
    	vec2 offset = snapToGrid(scaledDirection.xy);

    	float tangentAngle = 0.0;
    	float horizonAngle = findHorizonAngle(offset, NUM_SAMPLES, depth);
    	
    	ambientOcclusion += sin(horizonAngle) - sin(tangentAngle);
    }
    ambientOcclusion /= u_kernelSize;

    occlusion = vec3(ambientOcclusion);
}
