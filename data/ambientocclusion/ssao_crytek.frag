#version 150 core
#extension GL_ARB_explicit_attrib_location : require

in vec2 v_uv;
in vec3 v_viewRay;

uniform sampler2D u_normal_depth;
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

layout(location = 0) out float occlusion;

vec3 calcReflection()
{
    vec2 noise_scale = vec2(u_resolutionX / ROTATION_SIZE, u_resolutionY / ROTATION_SIZE);
    return texture(u_rotation, v_uv * noise_scale).xyz;
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
    vec3 reflection_normal = calcReflection();

    occlusion = 0.0;
    for (int i = 0; i < u_kernelSize; ++i)
    {
        vec3 reflected_kernel = reflect(kernel[i], reflection_normal);

        vec3 view_sample_point = position + reflected_kernel * u_kernelRadius;
        vec4 ndc_sample_point = u_proj * vec4(view_sample_point, 1.0);

        ndc_sample_point.xy /= ndc_sample_point.w;
        ndc_sample_point.xy *= 0.5;
        ndc_sample_point.xy += 0.5;

        // transform both depths to [0, u_farPlane]
        float linear_sample_depth = texture(u_normal_depth, ndc_sample_point.xy).a * u_farPlane;
        float linear_comp_depth = -view_sample_point.z;
        
        float occluded = linear_comp_depth > linear_sample_depth ? 1.0 : 0.0;
        
        if (u_attenuation)
        {
            // dividing by u_kernelRadius squared, because a bigger radius means that depths difference will be bigger
            float diff = 25 * (linear_comp_depth - linear_sample_depth) / (u_kernelRadius * u_kernelRadius);
            occlusion += (1.0 / (1.0 + diff * diff)) * occluded;
        }
        else
        {
            occlusion += 1.0 * occluded;
        }
    }

    occlusion /= u_kernelSize;
}
