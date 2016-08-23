#version 130

uniform sampler2D uiTexture;
in vec2 fragUV;
in vec4 fragColor;
out vec4 outColor;

void main() {
    outColor = fragColor * texture(uiTexture, fragUV.st).a;
}
