#version 150 core
#extension GL_ARB_explicit_attrib_location : require

in vec2 v_uv;

uniform sampler2D u_normal_depth;

uniform mat4 u_proj;
uniform mat4 u_invProj;
uniform int u_resolutionX;
uniform int u_resolutionY;

#define KERNEL_SIZE 32
uniform vec3 kernel[KERNEL_SIZE];

layout(location = 0) out float occlusion;

const float kernel_radius = 0.5;

void main()
{
    float depth = texture(u_normal_depth, v_uv).a;

    vec2 ndc = v_uv * 2.0 - 1.0;
    vec4 view_pos4 = u_invProj * vec4(ndc.x, ndc.y, depth, 1.0);
    vec3 view_pos = view_pos4.xyz / view_pos4.w;

    occlusion = 0.0;
    for (int i = 0; i < KERNEL_SIZE; ++i)
    {
        vec3 view_sample_point = view_pos + kernel[i] * kernel_radius;
        vec4 ndc_sample_point = u_proj * vec4(view_sample_point, 1.0);

        ndc_sample_point.xyz /= ndc_sample_point.w;
        ndc_sample_point.xy *= 0.5;
        ndc_sample_point.xy += 0.5;

        float sample_depth = texture(u_normal_depth, ndc_sample_point.xy).a;

        if (sample_depth <= ndc_sample_point.z) {
            occlusion += 1.0;
        }
    }
    
    occlusion /= KERNEL_SIZE;

    if (depth > 0.99) {
        occlusion = 0.0;
    }
}
