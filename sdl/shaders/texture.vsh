R"(
attribute vec4 vertex;
attribute vec2 uv;

uniform mat4 cameraMatrix;
uniform mat4 modelMatrix;

varying vec2 texCoord;

void main(void)
{
    gl_Position = cameraMatrix * modelMatrix * vertex;
    texCoord = uv;
}
)"
