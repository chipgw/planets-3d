#version 130

in vec4 vertex;
in vec3 tangent;
in vec2 uv;

uniform mat4 cameraMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

uniform sampler2D texture_height;

out vec2 texCoord;

out mat3 N;

void main() {
    vec4 V = vertex + vertex * 0.2 * vec4(texture(texture_height, uv).rgb - vec3(0.5), 0.0);
    gl_Position = cameraMatrix * modelMatrix * V;
    texCoord = uv;

    /* Create the view-space normal matrix. */
    vec3 n = normalize(viewMatrix * vec4(vertex.xyz, 0.0)).xyz;
    vec3 t = normalize(viewMatrix * vec4(tangent, 0.0)).xyz;
    N = mat3(t, -cross(n, t), n);
}
