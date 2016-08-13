attribute vec4 vertex;
attribute vec3 normal;
attribute vec3 tangent;
attribute vec2 uv;

uniform mat4 cameraMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

varying vec2 texCoord;

varying mat3 N;

void main() {
    gl_Position = cameraMatrix * modelMatrix * vertex;
    texCoord = uv;

    /* Create the view-space normal matrix. */
    vec3 n = normalize(viewMatrix * vec4(normal, 0.0)).xyz;
    vec3 t = normalize(viewMatrix * vec4(tangent, 0.0)).xyz;
    N = mat3(t, -cross(n, t), n);
}
