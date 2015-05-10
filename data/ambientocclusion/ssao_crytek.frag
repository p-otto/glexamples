#version 150 core
#extension GL_ARB_explicit_attrib_location : require

in vec2 v_uv;

uniform sampler2D u_normal_depth;
#define ROTATION_SIZE 4
uniform sampler2D u_rotation;

uniform mat4 u_proj;
uniform mat4 u_invProj;
uniform mat4 u_invView;
uniform int u_resolutionX;
uniform int u_resolutionY;

#define KERNEL_SIZE 32
uniform vec3 kernel[KERNEL_SIZE];

layout(location = 0) out float occlusion;

const float kernel_radius = 2.0;

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
    vec2 ndc = v_uv * 2.0 - 1.0;
    vec4 view_pos4 = u_invProj * vec4(ndc.x, ndc.y, depth, 1.0);
    return view_pos4.xyz / view_pos4.w;
}

void main()
{
    float depth = texture(u_normal_depth, v_uv).a * 2.0 - 1.0;
    vec3 normal = texture(u_normal_depth, v_uv).rgb;
    normal = normal * 2.0 - vec3(1.0);
    normal = normalize(normal);

    vec3 position = calcPosition(depth);
    mat3 tbn = calcRotatedTbn(normal);

    occlusion = 0.0;
    for (int i = 0; i < KERNEL_SIZE; ++i)
    {
        vec3 rotated_kernel = tbn * kernel[i];

        vec3 view_sample_point = position + rotated_kernel * kernel_radius;
        vec4 ndc_sample_point = u_proj * vec4(view_sample_point, 1.0);

        ndc_sample_point.xyz /= ndc_sample_point.w;
        ndc_sample_point.xy *= 0.5;
        ndc_sample_point.xy += 0.5;

        float sample_depth = texture(u_normal_depth, ndc_sample_point.xy).a * 2.0 - 1.0;

        if (sample_depth <= ndc_sample_point.z) {
            occlusion += 1.0;
        }
    }

    occlusion /= KERNEL_SIZE;

    if (depth > 0.99) {
        occlusion = 0.0;
    }
}
