uniform sampler2D texture_diff;
uniform sampler2D texture_nrm;

uniform highp vec3 lightDir;

varying highp vec2 texCoord;

varying highp mat3 N;

void main() {
    vec3 normal = N * (texture2D(texture_nrm, texCoord).rgb * 2.0 - 1.0);

    float light = max(dot(lightDir, normal), 0.0) + 0.1 + max(0.2 - normal.z * 0.2, 0.0);

    gl_FragColor = vec4(vec3(light) * texture2D(texture_diff, texCoord).rgb, 1.0);
}
