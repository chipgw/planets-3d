uniform sampler2D texture0;
varying highp vec2 texCoord0;

void main(void){
    gl_FragColor = texture2D(texture0, texCoord0);
}
