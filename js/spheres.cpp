#include <spheregenerator.h>
#include <grid.h>
#include <camera.h>
#include "glbindings.h"

int genSolid() {
    Sphere highResSphere(64, 32);
    glBufferData(GL_ARRAY_BUFFER, highResSphere.vertexCount * sizeof(Vertex), highResSphere.verts, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, highResSphere.triangleCount * sizeof(uint32_t), highResSphere.triangles, GL_STATIC_DRAW);
    return highResSphere.triangleCount;
}

int genWire() {
    Sphere lowResSphere(32, 16);
    glBufferData(GL_ARRAY_BUFFER, lowResSphere.vertexCount * sizeof(Vertex), lowResSphere.verts, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, lowResSphere.lineCount * sizeof(uint32_t), lowResSphere.lines, GL_STATIC_DRAW);
    return lowResSphere.lineCount;
}

int genCircle() {
    Circle circle(64);
    glBufferData(GL_ARRAY_BUFFER, circle.vertexCount * sizeof(glm::vec3), circle.verts, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, circle.lineCount * sizeof(uint32_t), circle.lines, GL_STATIC_DRAW);
    return circle.lineCount;
}

EMSCRIPTEN_BINDINGS(sphere) {
    emscripten::function("genSolidSphere",  &genSolid);
    emscripten::function("genWireSphere",   &genWire);
    emscripten::function("genCircle",       &genCircle);
}
