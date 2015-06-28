#version 150 core
#extension GL_ARB_explicit_attrib_location : require
#extension GL_ARB_shading_language_include : require

in vec2 v_uv;
in vec3 v_viewRay;

uniform sampler2D u_normal_depth;
uniform sampler2D u_direct_light;
#define ROTATION_SIZE 4
uniform sampler2D u_rotation;

uniform mat4 u_proj;
uniform float u_farPlane;
uniform int u_resolutionX;
uniform int u_resolutionY;
uniform float u_kernelRadius;
uniform int u_kernelSize;
uniform bool u_attenuation;

#define MAX_KERNEL_SIZE 128
uniform vec3 kernel[MAX_KERNEL_SIZE];

layout(location = 0) out vec3 color;

const float color_bleeding_strength = 3.0;

#include "/utility"

void main()
{
    float depth = texture(u_normal_depth, v_uv).a;
    vec3 normal = texture(u_normal_depth, v_uv).rgb * 2.0 - vec3(1.0);
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

        // transform both depths to [0, u_farPlane]
        float linear_sample_depth = texture(u_normal_depth, ndc_sample_point.xy).a * u_farPlane;
        float linear_comp_depth = -view_sample_point.z;

        float occluded = linear_comp_depth > linear_sample_depth ? 1.0 : 0.0;

        // calculate possible color bleeding
        vec3 transmittance_direction = normalize(position - view_sample_point);
        vec3 sender_normal = texture(u_normal_depth, ndc_sample_point.xy).rgb * 2.0 - 1.0;
        float sender_transmittance_cos = max(0.0, dot(sender_normal, transmittance_direction));
        float receiver_transmittance_cos = max(0.0, dot(normal, -transmittance_direction));
        // TODO: factor in attenuation
        //float dist = abs(linear_comp_depth - linear_sample_depth);

        color_bleeding += color_bleeding_strength * texture(u_direct_light, ndc_sample_point.xy).rgb * occluded * sender_transmittance_cos * receiver_transmittance_cos;
    }

    color_bleeding /= u_kernelSize;

    if (depth > 0.99)
    {
        color_bleeding = vec3(0.0);
    }

    // add original color and accumulated color bleeding
    color = texture(u_direct_light, v_uv).rgb + color_bleeding;
}
