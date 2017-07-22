uniform sampler2D texture_diff;
uniform sampler2D texture_nrm;

uniform highp vec3 lightDir;

varying highp vec2 texCoord;
varying highp mat3 N;
varying highp vec3 pos;

#define M_PI 3.1415926535897932384626433832795

void main() {
    vec2 uv = vec2(atan(pos.x, pos.y) / (2*M_PI) + 0.5, texCoord.y);
    vec3 normal = N * (texture2D(texture_nrm, uv).rgb * 2.0 - 1.0);

    float light = max(dot(lightDir, normal), 0.0) + 0.1 + max(0.2 - normal.z * 0.2, 0.0);

    gl_FragColor = vec4(vec3(light) * texture2D(texture_diff, uv).rgb, 1.0);
}
