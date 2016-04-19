#include <spheregenerator.h>
#include <grid.h>
#include <camera.h>
#include "glbindings.h"

class Spheres {
    GLuint highResVBO, highResTriIBO, highResTriCount;
    GLuint lowResVBO, lowResLineIBO, lowResLineCount;
    GLuint circleVBO, circleLineIBO, circleLineCount;

public:
    Spheres();

    void bindSolid();
    void bindWire();
    void bindCircle();

    void drawSolid();
    void drawWire();
    void drawCircle();
};

Spheres::Spheres() {
    Sphere<64, 32> highResSphere;
    Sphere<32, 16> lowResSphere;
    Circle<64> circle;

    glGenBuffers(1, &highResVBO);
    glBindBuffer(GL_ARRAY_BUFFER, highResVBO);
    glBufferData(GL_ARRAY_BUFFER, highResSphere.vertexCount * sizeof(Vertex), highResSphere.verts, GL_STATIC_DRAW);

    glGenBuffers(1, &highResTriIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, highResTriIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, highResSphere.triangleCount * sizeof(uint32_t), highResSphere.triangles, GL_STATIC_DRAW);

    glGenBuffers(1, &lowResVBO);
    glBindBuffer(GL_ARRAY_BUFFER, lowResVBO);
    glBufferData(GL_ARRAY_BUFFER, lowResSphere.vertexCount * sizeof(Vertex), lowResSphere.verts, GL_STATIC_DRAW);

    glGenBuffers(1, &lowResLineIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lowResLineIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, lowResSphere.lineCount * sizeof(uint32_t), lowResSphere.lines, GL_STATIC_DRAW);

    glGenBuffers(1, &circleVBO);
    glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
    glBufferData(GL_ARRAY_BUFFER, circle.vertexCount * sizeof(Vertex), circle.verts, GL_STATIC_DRAW);

    glGenBuffers(1, &circleLineIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, circleLineIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, circle.lineCount * sizeof(uint32_t), circle.lines, GL_STATIC_DRAW);

    highResTriCount = highResSphere.triangleCount;
    lowResLineCount = lowResSphere.lineCount;
    circleLineCount = circle.lineCount;

    glEnableVertexAttribArray(vertex);
}

void Spheres::bindSolid() {
    glBindBuffer(GL_ARRAY_BUFFER, highResVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, highResTriIBO);

    glEnableVertexAttribArray(uv);
    glVertexAttribPointer(vertex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glVertexAttribPointer(uv,     2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
}

void Spheres::bindWire() {
    glBindBuffer(GL_ARRAY_BUFFER, lowResVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lowResLineIBO);

    glDisableVertexAttribArray(uv);
    glVertexAttribPointer(vertex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
}

void Spheres::bindCircle() {
    glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, circleLineIBO);

    glDisableVertexAttribArray(uv);
    glVertexAttribPointer(vertex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
}

void Spheres::drawSolid() {
    glDrawElements(GL_TRIANGLES, highResTriCount, GL_UNSIGNED_INT, 0);
}

void Spheres::drawWire() {
    glDrawElements(GL_LINES, lowResLineCount, GL_UNSIGNED_INT, 0);
}

void Spheres::drawCircle() {
    glDrawElements(GL_LINES, circleLineCount, GL_UNSIGNED_INT, 0);
}

EMSCRIPTEN_BINDINGS(sphere) {
    emscripten::class_<Spheres>("Spheres")
            .constructor()
            .function("bindSolid",  &Spheres::bindSolid)
            .function("bindWire",   &Spheres::bindWire)
            .function("bindCircle", &Spheres::bindCircle)
            .function("drawSolid",  &Spheres::drawSolid)
            .function("drawWire",   &Spheres::drawWire)
            .function("drawCircle", &Spheres::drawCircle)
            ;
}
