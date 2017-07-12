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

EMSCRIPTEN_BINDINGS(sphere) {
    emscripten::function("genSolidSphere",  &genSolid);
    emscripten::function("genWireSphere",   &genWire);
    emscripten::function("genCircle",       &genCircle);
}
