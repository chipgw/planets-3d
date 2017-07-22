attribute highp vec4 vertex;
attribute highp vec3 tangent;
attribute highp vec2 uv;

uniform highp mat4 cameraMatrix;
uniform highp mat4 viewMatrix;
uniform highp mat4 modelMatrix;

varying highp vec2 texCoord;
varying highp mat3 N;
varying highp vec3 pos;

void main() {
    gl_Position = cameraMatrix * modelMatrix * vertex;
    texCoord = uv;
    pos = vertex.xyz;

    /* Create the view-space normal matrix. */
    vec3 n = normalize(viewMatrix * vec4(vertex.xyz, 0.0)).xyz;
    vec3 t = normalize(viewMatrix * vec4(tangent, 0.0)).xyz;
    N = mat3(t, -cross(n, t), n);
}
