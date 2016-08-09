attribute highp vec4 vertex;
attribute highp vec3 normal;
attribute highp vec3 tangent;
attribute highp vec2 uv;

uniform highp mat4 cameraMatrix;
uniform highp mat4 viewMatrix;
uniform highp mat4 modelMatrix;

varying highp vec2 texCoord;

varying highp mat3 N;

void main() {
    gl_Position = cameraMatrix * modelMatrix * vertex;
    texCoord = uv;

    /* Create the view-space normal matrix. */
    vec3 n = normalize(viewMatrix * vec4(normal, 0.0)).xyz;
    vec3 t = normalize(viewMatrix * vec4(tangent, 0.0)).xyz;
    N = mat3(t, -cross(n, t), n);
}
