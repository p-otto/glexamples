#version 150 core
#extension GL_ARB_explicit_attrib_location : require
#extension GL_ARB_shading_language_include : require

in vec2 v_uv;
in vec3 v_viewRay;

uniform sampler2D u_normal_depth;
#define ROTATION_SIZE 4
uniform sampler2D u_rotation;

uniform mat4 u_proj;
uniform mat4 u_view;
uniform float u_farPlane;
uniform int u_resolutionX;
uniform int u_resolutionY;
uniform float u_kernelRadius;
uniform int u_kernelSize;

#define MAX_KERNEL_SIZE 128
uniform vec3 kernel[MAX_KERNEL_SIZE];

layout(location = 0) out vec3 color;

const vec3 ambient_color = vec3(1.0);

mat3 calcRotatedTbn(vec3 normal)
{
    vec2 noise_scale = vec2(u_resolutionX / ROTATION_SIZE, u_resolutionY / ROTATION_SIZE);
    vec3 rvec = texture(u_rotation, v_uv * noise_scale).xyz;
    vec3 tangent = normalize(rvec - normal * dot(rvec, normal));
    vec3 bitangent = cross(normal, tangent);
    return mat3(tangent, bitangent, normal);
}

mat3 calcTbn(vec3 normal)
{
    vec3 rvec = vec3(0, 1, 0);
    vec3 tangent = normalize(rvec - normal * dot(rvec, normal));
    vec3 bitangent = cross(normal, tangent);
    return mat3(tangent, bitangent, normal);
}

vec3 calcPosition(float depth)
{
    return v_viewRay * depth;
}

void main()
{
    float depth = texture(u_normal_depth, v_uv).a;
    vec3 normal = texture(u_normal_depth, v_uv).rgb * 2.0 - vec3(1.0);
    normal = normalize(normal);

    vec3 position = calcPosition(depth);
    mat3 tbn = calcRotatedTbn(normal);

    vec3 occlusion = vec3(0.0);
    for (int i = 0; i < u_kernelSize; ++i)
    {
        vec3 rotated_kernel = tbn * kernel[i];

        vec3 view_sample_point = position + rotated_kernel * u_kernelRadius;
        vec4 ndc_sample_point = u_proj * vec4(view_sample_point, 1.0);

        ndc_sample_point.xy /= ndc_sample_point.w;
        ndc_sample_point.xy *= 0.5;
        ndc_sample_point.xy += 0.5;

        if (ndc_sample_point.x > 1 || ndc_sample_point.y > 1 || ndc_sample_point.x < 0 || ndc_sample_point.y < 0)
        {
            occlusion += ambient_color;
            continue;
        }

        // transform both depths to [0, u_farPlane]
        float linear_sample_depth = texture(u_normal_depth, ndc_sample_point.xy).a * u_farPlane;
        float linear_comp_depth = -view_sample_point.z;

        float occluded = linear_comp_depth > linear_sample_depth ? 1.0 : 0.0;

        // direct lighting can be calculated from an environment map, point light, etc.
        // here, a environment map is simulated by using the up vector as the maximal ambient light
        vec3 position_to_sample = normalize(view_sample_point - position);
        occlusion += ambient_color * (1.0 - occluded) * clamp(dot(position_to_sample, vec3(0.0, 1.0, 0.0)) + 1.0, 0.0, 1.0);
    }

    occlusion /= u_kernelSize;

    if (depth > 0.99)
    {
        occlusion = vec3(0.0);
    }

    color = occlusion;
}
