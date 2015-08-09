#define ROTATION_SIZE 4

const float pi = 3.14159265;

vec3 sampleNoiseTexture(sampler2D noise, vec2 uv, float resX, float resY)
{
    vec2 noise_scale = vec2(resX / ROTATION_SIZE, resY / ROTATION_SIZE);
    return texture(noise, uv * noise_scale).xyz;
}

mat3 calcRotatedTbn(sampler2D noise, vec3 normal, vec2 uv, float resX, float resY)
{
    vec3 rvec = sampleNoiseTexture(noise, uv, resX, resY);
    vec3 tangent = normalize(rvec - normal * dot(rvec, normal));
    vec3 bitangent = cross(normal, tangent);
    return mat3(tangent, bitangent, normal);
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

vec2 rotate(vec2 direction, vec2 rand)
{
    return vec2(direction.x*rand.x - direction.y*rand.y,
                  direction.x*rand.y + direction.y*rand.x);
}