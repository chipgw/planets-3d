#version 130

uniform mat4 matrix;
in vec2 vertex;
in vec2 uv;
/* The shader system doesn't have a color attribute, so we just hijack the tangent attribute. */
in vec4 tangent;

out vec2 fragUV;
out vec4 fragColor;

void main() {
    fragUV = uv;
    fragColor = tangent;
    gl_Position = matrix * vec4(vertex.xy, 0.0, 1.0);
}
