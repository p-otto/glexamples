#version 150 core
#extension GL_ARB_explicit_attrib_location : require

in vec2 v_uv;

uniform sampler2D u_normal_depth;
uniform sampler2D u_occlusion;
uniform bool u_biliteral;
uniform int u_kernelSize;

layout(location = 0) out vec3 occlusion;

#define EPSILON 0.001
#define COMP_DEPTH 0.01
#define COMP_NORMAL 0.5

void main()
{
    vec4 normal_depth = texture(u_normal_depth, v_uv);

    occlusion = vec3(0.0);
    float count = 0.0;
    for (int x = -u_kernelSize; x <= u_kernelSize; ++x)
    {
        vec2 cur_uv = v_uv + vec2(x * EPSILON, 0);
        
        if (u_biliteral)
        {
            vec4 cur_normal_depth = texture(u_normal_depth, cur_uv);
            
            float depth_test = abs(normal_depth.a - cur_normal_depth.a) < COMP_DEPTH ? 1.0 : 0.0;
            float normal_test = dot(normal_depth.xyz * 2.0 - vec3(1.0), cur_normal_depth.xyz * 2.0 - vec3(1.0)) > COMP_NORMAL ? 1.0 : 0.0;
            occlusion += texture(u_occlusion, cur_uv).rgb * depth_test * normal_test;
            count += depth_test * normal_test;
        }
        else {
            occlusion += texture(u_occlusion, cur_uv).rgb;
            count += 1.0;
        }
    }
    occlusion /= count;
}
