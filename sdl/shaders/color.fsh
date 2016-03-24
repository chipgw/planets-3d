#ifdef EMSCRIPTEN
"precision highp float;"
#endif
R"(
uniform vec4 color;

void main() {
    gl_FragColor = color;
}
)"
