#version 130

#define M_PI 3.1415926535897932384626433832795

uniform sampler2D texture_diff;
uniform sampler2D texture_nrm;

uniform vec3 lightDir;

in vec2 texCoord;
in mat3 N;
in vec3 pos;

out vec4 outColor;

void main() {
    vec2 uv = vec2(atan(pos.x, pos.y) / (2*M_PI) + 0.5, texCoord.y);
    vec3 normal = N * (texture(texture_nrm, uv).rgb * 2.0 - 1.0);

    float light = max(dot(lightDir, normal), 0.0) + 0.1 + max(0.2 - normal.z * 0.2, 0.0);

    outColor = vec4(vec3(light) * texture(texture_diff, uv).rgb, 1.0);
}
