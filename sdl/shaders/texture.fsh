#version 130

uniform sampler2D texture_diff;
uniform sampler2D texture_nrm;

uniform vec3 lightDir;

in vec2 texCoord;
in mat3 N;

out vec4 outColor;

void main() {
    vec3 normal = N * (texture(texture_nrm, texCoord).rgb * 2.0 - 1.0);

    float light = max(dot(lightDir, normal), 0.0) + 0.1 + max(0.2 - normal.z * 0.2, 0.0);

    outColor = vec4(vec3(light) * texture(texture_diff, texCoord).rgb, 1.0);
}
