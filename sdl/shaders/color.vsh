R"(
attribute vec4 vertex;

uniform mat4 cameraMatrix;
uniform mat4 modelMatrix;

void main() {
    gl_Position = cameraMatrix * modelMatrix * vertex;
}
)"
