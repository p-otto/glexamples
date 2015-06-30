#version 150 core
#extension GL_ARB_explicit_attrib_location : require
#extension GL_ARB_shading_language_include : require

in vec2 v_uv;
in vec3 v_viewRay;

uniform sampler2D u_normal_depth;
#define ROTATION_SIZE 4
uniform sampler2D u_rotation;

uniform mat4 u_proj;
uniform float u_nearPlane;
uniform float u_farPlane;
uniform int u_resolutionX;
uniform int u_resolutionY;
uniform float u_kernelRadius;
uniform int u_kernelSize;

#define MAX_KERNEL_SIZE 128
uniform vec3 kernel[MAX_KERNEL_SIZE];

layout(location = 0) out vec3 occlusion;

#include "/utility"

void main()
{
    float depth = texture(u_normal_depth, v_uv).a;
    vec3 normal = texture(u_normal_depth, v_uv).rgb * 2.0 - vec3(1.0);
    normal = normalize(normal);

    vec3 position = calcPosition(v_viewRay, depth);
    vec3 reflection_normal = calcReflection(u_rotation, v_uv, u_resolutionX, u_resolutionY);

    float occlusion_factor = 0.0;
    for (int i = 0; i < u_kernelSize; ++i)
    {
        vec3 reflected_kernel = reflect(kernel[i], reflection_normal);

        vec3 view_sample_point = position + reflected_kernel * u_kernelRadius;
        vec4 ndc_sample_point = u_proj * vec4(view_sample_point, 1.0);

        ndc_sample_point.xy /= ndc_sample_point.w;
        ndc_sample_point.xy *= 0.5;
        ndc_sample_point.xy += 0.5;

        if (ndc_sample_point.x > 1 || ndc_sample_point.y > 1 || ndc_sample_point.x < 0 || ndc_sample_point.y < 0)
        {
            occlusion_factor += 0.5 / u_kernelSize;
            continue;
        }

        // transform both depths to [u_nearPlane, u_farPlane]
        float linear_sample_depth = texture(u_normal_depth, ndc_sample_point.xy).a * (u_farPlane - u_nearPlane) + u_nearPlane;
        float linear_comp_depth = -view_sample_point.z;

        // use 0.5 if check fails, because crytek ssao is gray on average due to sphere sampling
        float range_check = abs(linear_comp_depth - linear_sample_depth) < u_kernelRadius ? 1.0 : 0.5;
        float occluded = linear_comp_depth > linear_sample_depth ? 1.0 : 0.0;

        occlusion_factor += 1.0 * occluded * range_check;
    }

    occlusion_factor /= u_kernelSize;
    occlusion_factor = 1.0 - occlusion_factor;

    if (depth > 0.99)
    {
        occlusion_factor = 0.0;
    }

    occlusion = vec3(occlusion_factor);
}
