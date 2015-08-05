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
uniform int u_lengthDistribution;

const int linear = 0;
const int quadratic = 1;
const int starcraft = 2;

ivec2 resolution = ivec2(u_resolutionX, u_resolutionY);

layout(location = 0) out vec3 occlusion;

#include "/utility"

float findHorizonAngle(vec2 offset, int numSamples, float depth, float randomOffset)
{
    // jitter starting offset
    vec2 startOffset = offset * randomOffset;

    float largestHorizonAngle = -pi / 2;

    for (int step = 0; step < numSamples; step++) 
    {
        float sampleDistance = float(step) / numSamples;
        switch (u_lengthDistribution)
        {
            case quadratic:
                sampleDistance *= sampleDistance;
                break;
            case starcraft:
                if (step > 3 * float(numSamples) / 4) {
                    sampleDistance *= 4;
                }
                sampleDistance *= sampleDistance;
                break;
        }
        vec2 sampleOffset = snapToGrid(startOffset + offset * sampleDistance, resolution);
        float sampleDepth = texture(u_normal_depth, v_uv + sampleOffset).a;
        float horizonAngle = atan(depth - sampleDepth, length(sampleOffset));

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

    vec3 position = calcPosition(v_viewRay, depth);

    vec3 dx = dFdx(position);
    vec3 dy = dFdy(position);

    float ambientOcclusion = 0.0;

    vec3 random = sampleNoiseTexture(u_rotation, v_uv, u_resolutionX, u_resolutionY);

    // compute average occlusion over serveral directions
    for (int i = 0; i < u_numDirections; i++)
    {
        // compute randomly rotated sample texture
        float angle = i * (2 * pi / u_numDirections);
        vec2 sampleDirection = vec2(sin(angle), cos(angle));
        sampleDirection = rotate(sampleDirection, random.xy);
        sampleDirection = normalize(sampleDirection);

        // compute view dependent sample vector
        vec4 scaledDirection = u_proj * vec4(position + vec3(sampleDirection * u_kernelRadius, 0.0), 1.0);
        scaledDirection /= scaledDirection.w;
        scaledDirection = scaledDirection * 0.5 + 0.5;
        scaledDirection -= vec4(v_uv, 0.0, 0.0);

        vec2 offset = scaledDirection.xy;

        float tangentAngle = findTangentAngle(offset, dx, dy);
        float horizonAngle = findHorizonAngle(offset, u_numSamples, depth, random.z);
        
        ambientOcclusion += sin(horizonAngle) - sin(tangentAngle);
    }
    ambientOcclusion /= u_numDirections;

    occlusion = 1 - vec3(ambientOcclusion);
}
