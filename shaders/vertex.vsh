attribute highp vec4 vertex;
attribute highp vec2 texCoord;

uniform highp mat4 cameraMatrix;
uniform highp mat4 modelMatrix;

varying highp vec2 texCoord0;

void main(void)
{
    gl_Position = cameraMatrix * modelMatrix * vertex;
    texCoord0 = texCoord;
}
