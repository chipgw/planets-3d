#include <spheregenerator.h>
#include <grid.h>
#include <camera.h>
#include "glbindings.h"

int genSolid() {
    Sphere<64, 32> highResSphere;
    glBufferData(GL_ARRAY_BUFFER, highResSphere.vertexCount * sizeof(Vertex), highResSphere.verts, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, highResSphere.triangleCount * sizeof(uint32_t), highResSphere.triangles, GL_STATIC_DRAW);
    return highResSphere.triangleCount;
}

int genWire() {
    Sphere<32, 16> lowResSphere;
    glBufferData(GL_ARRAY_BUFFER, lowResSphere.vertexCount * sizeof(Vertex), lowResSphere.verts, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, lowResSphere.lineCount * sizeof(uint32_t), lowResSphere.lines, GL_STATIC_DRAW);
    return lowResSphere.lineCount;
}

int genCircle() {
    Circle<64> circle;
    glBufferData(GL_ARRAY_BUFFER, circle.vertexCount * sizeof(glm::vec3), circle.verts, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, circle.lineCount * sizeof(uint32_t), circle.lines, GL_STATIC_DRAW);
    return circle.lineCount;
}

void drawArrow(float length) {
    float verts[] = {  0.1f, 0.1f, 0.0f,
                       0.1f,-0.1f, 0.0f,
                      -0.1f,-0.1f, 0.0f,
                      -0.1f, 0.1f, 0.0f,

                       0.1f, 0.1f, length,
                       0.1f,-0.1f, length,
                      -0.1f,-0.1f, length,
                      -0.1f, 0.1f, length,

                       0.2f, 0.2f, length,
                       0.2f,-0.2f, length,
                      -0.2f,-0.2f, length,
                      -0.2f, 0.2f, length,

                       0.0f, 0.0f, length + 0.4f };

    static const GLubyte indexes[] = {  0,  1,  2,       2,  3,  0,

                                        1,  0,  5,       4,  5,  0,
                                        2,  1,  6,       5,  6,  1,
                                        3,  2,  7,       6,  7,  2,
                                        0,  3,  4,       7,  4,  3,

                                        5,  4,  9,       8,  9,  4,
                                        6,  5, 10,       9, 10,  5,
                                        7,  6, 11,      10, 11,  6,
                                        4,  7,  8,      11,  8,  7,

                                        9,  8, 12,
                                       10,  9, 12,
                                       11, 10, 12,
                                        8, 11, 12 };

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glEnableVertexAttribArray(vertex);

    glVertexAttribPointer(vertex, 3, GL_FLOAT, GL_FALSE, 0, verts);
    glDrawElements(GL_TRIANGLES, sizeof(indexes), GL_UNSIGNED_BYTE, indexes);
}

EMSCRIPTEN_BINDINGS(sphere) {
    emscripten::function("genSolidSphere",  &genSolid);
    emscripten::function("genWireSphere",   &genWire);
    emscripten::function("genCircle",       &genCircle);
    emscripten::function("drawArrow",       &drawArrow);
}
