#version 150 core
#extension GL_ARB_explicit_attrib_location : require
#extension GL_ARB_shading_language_include : require

in vec2 v_uv;
in vec3 v_viewRay;

uniform sampler2D u_normal_depth;
uniform sampler2D u_direct_light;
uniform sampler2D u_ambient;
uniform sampler2D u_diffuse;
#define ROTATION_SIZE 4
uniform sampler2D u_rotation;

uniform mat4 u_proj;
uniform mat4 u_invProj;
uniform float u_nearPlane;
uniform float u_farPlane;
uniform int u_resolutionX;
uniform int u_resolutionY;
uniform float u_kernelRadius;
uniform int u_kernelSize;
uniform bool u_attenuation;

#define MAX_KERNEL_SIZE 128
uniform vec3 kernel[MAX_KERNEL_SIZE];

layout(location = 0) out vec3 color;

const float color_bleeding_strength = 1.0;

#include "/utility"

void main()
{
    float depth = texture(u_normal_depth, v_uv).a;
    vec3 normal = texture(u_normal_depth, v_uv).rgb * 2.0 - 1.0;
    normal = normalize(normal);

    vec3 position = calcPosition(v_viewRay, depth);
    mat3 tbn = calcRotatedTbn(u_rotation, normal, v_uv, u_resolutionX, u_resolutionY);

    vec3 color_bleeding = vec3(0.0);
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
            continue;
        }

        // transform both depths to [u_nearPlane, u_farPlane]
        float linear_sample_depth = texture(u_normal_depth, ndc_sample_point.xy).a * (u_farPlane - u_nearPlane) + u_nearPlane;
        float linear_comp_depth = -view_sample_point.z;

        float occluded = linear_comp_depth > linear_sample_depth ? 1.0 : 0.0;

        // calculate sender position in view space
        vec4 sender_ndc = vec4(ndc_sample_point.xy * 2.0 - 1.0, linearDepthToLogDepth(linear_sample_depth, u_proj) * 2.0 - 1.0, 1.0);
        sender_ndc = u_invProj * sender_ndc;
        vec3 sender_position = sender_ndc.xyz / sender_ndc.w;

        // calculate angles between transmission direction and the both normals
        vec3 transmittance_direction = normalize(position - sender_position);
        vec3 sender_normal = normalize(texture(u_normal_depth, ndc_sample_point.xy).rgb * 2.0 - 1.0);
        float sender_transmittance_cos = max(0.0, dot(sender_normal, transmittance_direction));
        float receiver_transmittance_cos = max(0.0, dot(normal, -transmittance_direction));
        // TODO: factor in attenuation
        float dist = abs(linear_comp_depth - linear_sample_depth);

        vec3 ambient = texture(u_ambient, ndc_sample_point.xy).rgb;
        vec3 diffuse = texture(u_diffuse, ndc_sample_point.xy).rgb;
        vec3 direct_light = texture(u_direct_light, ndc_sample_point.xy).rgb;

        vec3 cur_color_bleeding = color_bleeding_strength * evaluatePhong(ambient, diffuse, direct_light) * (u_kernelRadius * u_kernelRadius) * occluded * sender_transmittance_cos * receiver_transmittance_cos;
        cur_color_bleeding /= max(1.0, dist * dist);
        color_bleeding += clamp(cur_color_bleeding, 0.0, 1.0);
    }

    color_bleeding /= u_kernelSize;

    if (depth > 0.99)
    {
        color_bleeding = vec3(0.0);
    }

    // add original color and accumulated color bleeding
    color = texture(u_direct_light, v_uv).rgb + color_bleeding;
}