#version 150 core
#extension GL_ARB_explicit_attrib_location : require

in vec2 v_uv;
in vec3 v_viewRay;

uniform sampler2D u_normal_depth;
#define ROTATION_SIZE 4
uniform sampler2D u_rotation;

uniform float u_kernelRadius;
uniform mat4 u_proj;
uniform int u_resolutionX;
uniform int u_resolutionY;
uniform int u_numDirections;
uniform int u_numSamples;

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

float findHorizonAngle(vec2 startOffset, vec2 offset, int numSamples, float depth)
{
	float largestHorizonAngle = 0.0;
	for (int step = 1; step < numSamples; step++) {
		vec2 sampleOffset = snapToGrid(startOffset + offset * step);
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

    vec3 dx = dFdx(position);
    vec3 dy = dFdy(position);

    float ambientOcclusion = 0.0;
    float stepSize = u_kernelRadius / NUM_SAMPLES;

    vec2 noise_scale = vec2(u_resolutionX / ROTATION_SIZE, u_resolutionY / ROTATION_SIZE);
    vec3 random = texture(u_rotation, v_uv * noise_scale).xyz;

    for (int i = 0; i < NUM_DIRECTIONS; i++) {
    	float angle = i * (2 * pi / NUM_DIRECTIONS);
    	vec2 sampleDirection = rotate(vec2(sin(angle), cos(angle)), random.xy);
    	sampleDirection = normalize(sampleDirection);
    	vec4 scaledDirection = u_proj * vec4(position + vec3(sampleDirection * stepSize, 0.0), 1.0);
    	
    	scaledDirection /= scaledDirection.w;
    	scaledDirection = scaledDirection * 0.5 + 0.5;

    	scaledDirection -= vec4(v_uv, 0.0, 0.0);
    	vec2 startOffset = snapToGrid(scaledDirection.xy * random.z);
    	vec2 offset = scaledDirection.xy;

    	float tangentAngle = findTangentAngle(offset, dx, dy);
    	float horizonAngle = findHorizonAngle(startOffset, offset, NUM_SAMPLES, depth);
    	
    	ambientOcclusion += sin(horizonAngle) - sin(tangentAngle);
    }
    ambientOcclusion /= NUM_DIRECTIONS;

    occlusion = vec3(ambientOcclusion);
}
