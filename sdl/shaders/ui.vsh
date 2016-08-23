#version 130

uniform mat4 matrix;
in vec2 vertex;
in vec2 uv;
/* The shader system doesn't have a color attribute, so we just hijack the normal attribute. */
in vec4 normal;

out vec2 fragUV;
out vec4 fragColor;

void main() {
    fragUV = uv;
    fragColor = normal;
    gl_Position = matrix * vec4(vertex.xy, 0.0, 1.0);
}
