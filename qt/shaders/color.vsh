attribute highp vec4 vertex;

uniform highp mat4 cameraMatrix;
uniform highp mat4 modelMatrix;

void main() {
    gl_Position = cameraMatrix * modelMatrix * vertex;
}
