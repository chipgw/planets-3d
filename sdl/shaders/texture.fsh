R"(
uniform sampler2D texture;
varying vec2 texCoord;

void main(void){
    gl_FragColor = texture2D(texture, texCoord);
}
)"
