#define ROTATION_SIZE 4

mat3 calcRotatedTbn(sampler2D noise, vec3 normal, vec2 uv, float resX, float resY)
{
    vec2 noise_scale = vec2(resX / ROTATION_SIZE, resY / ROTATION_SIZE);
    vec3 rvec = texture(noise, uv * noise_scale).xyz;
    vec3 tangent = normalize(rvec - normal * dot(rvec, normal));
    vec3 bitangent = cross(normal, tangent);
    return mat3(tangent, bitangent, normal);
}

vec3 calcReflection(sampler2D noise, vec2 uv, float resX, float resY)
{
    vec2 noise_scale = vec2(resX / ROTATION_SIZE, resY / ROTATION_SIZE);
    return texture(noise, uv * noise_scale).xyz;
}

vec3 calcPosition(vec3 ray, float depth)
{
    return ray * depth;
}

vec3 evaluatePhong(vec3 ambient, vec3 diffuse, vec3 occlusion)
{
    return ambient * occlusion + diffuse;
}

float linearDepthToLogDepth(float depth, mat4 proj)
{
    float A = proj[2].z;
    float B = proj[3].z;
    return 0.5*(-A*depth + B) / depth + 0.5;
}
