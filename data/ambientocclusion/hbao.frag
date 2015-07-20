#version 150 core
#extension GL_ARB_explicit_attrib_location : require
#extension GL_ARB_shading_language_include : require

in vec2 v_uv;
in vec3 v_viewRay;

uniform sampler2D u_normal_depth;
uniform sampler2D u_rotation;

uniform float u_kernelRadius;
uniform mat4 u_proj;
uniform int u_resolutionX;
uniform int u_resolutionY;
uniform int u_numDirections;
uniform int u_numSamples;

ivec2 resolution = ivec2(u_resolutionX, u_resolutionY);

layout(location = 0) out vec3 occlusion;

const float pi = 3.14159265;

#include "/utility"

float findHorizonAngle(vec2 startOffset, vec2 offset, int numSamples, float depth)
{
	float largestHorizonAngle = -pi / 2;
	for (int step = 1; step <= numSamples; step++) {
		float sampleDistance = step / numSamples;
        vec2 sampleOffset = snapToGrid(startOffset + offset * sampleDistance, resolution);
		float sampleDepth = texture(u_normal_depth, v_uv + sampleOffset).a;
		float horizonAngle = atan((sampleDepth - depth)/ length(sampleOffset));

		largestHorizonAngle = max(largestHorizonAngle, horizonAngle);
	}
	return largestHorizonAngle;
}

float findTangentAngle(vec2 offset, vec3 dx, vec3 dy)
{
	vec3 T = offset.x * dx + offset.y * dy;
	float tangentAngle = atan(T.z / length(T.xy));

	return tangentAngle;
}

void main()
{
    float depth = texture(u_normal_depth, v_uv).a;
    vec3 normal = texture(u_normal_depth, v_uv).rgb * 2.0 - vec3(1.0);
    normal = normalize(normal);

    vec3 position = calcPosition(v_viewRay, depth);

    vec3 dx = dFdx(position);
    vec3 dy = dFdy(position);

    float ambientOcclusion = 0.0;
    float stepSize = u_kernelRadius / u_numSamples;

    vec2 noise_scale = vec2(u_resolutionX / ROTATION_SIZE, u_resolutionY / ROTATION_SIZE);
    vec3 random = texture(u_rotation, v_uv * noise_scale).xyz;

    for (int i = 0; i < u_numDirections; i++) {
    	float angle = i * (2 * pi / u_numDirections);
    	vec2 sampleDirection = vec2(sin(angle), cos(angle));
    	sampleDirection = rotate(sampleDirection, random.xy);
    	sampleDirection = normalize(sampleDirection);

    	vec4 scaledDirection = u_proj * vec4(position + vec3(sampleDirection * u_kernelRadius, 0.0), 1.0);
    	scaledDirection /= scaledDirection.w;
    	scaledDirection = scaledDirection * 0.5 + 0.5;
    	scaledDirection -= vec4(v_uv, 0.0, 0.0);

        vec2 startOffset = snapToGrid(scaledDirection.xy * random.z, resolution);
    	vec2 offset = scaledDirection.xy;

    	float tangentAngle = findTangentAngle(offset, dx, dy);
    	float horizonAngle = findHorizonAngle(startOffset, offset, u_numSamples, depth);
    	
    	ambientOcclusion += sin(horizonAngle) - sin(tangentAngle);
    }
    ambientOcclusion /= u_numDirections;

    occlusion = vec3(ambientOcclusion) + 1;
}
