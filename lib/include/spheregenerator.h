#pragma once

#include "types.h"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

struct Vertex {
    glm::vec3 position;
    glm::vec3 tangent;
    glm::vec2 uv;
};

class Circle {
public:
    const uint32_t vertexCount;
    const uint32_t lineCount;

    glm::vec3* verts;
    uint32_t* lines;

    EXPORT Circle(uint32_t slices);
    EXPORT ~Circle();
};

class Sphere {
public:
    const uint32_t vertexCount;
    const uint32_t triangleCount;
    const uint32_t lineCount;

    Vertex* verts;
    uint32_t* triangles;
    uint32_t* lines;

    EXPORT Sphere(uint32_t slices, uint32_t stacks);
    EXPORT ~Sphere();
};

class IcoSphere {
    void subdivide(uint8_t divisions);
    void initNoSubdiv();

public:
    const uint32_t vertexCount;
    const uint32_t triangleCount;
    const uint32_t lineCount;

    Vertex* verts;
    uint32_t* triangles;
    uint32_t* lines;

    EXPORT IcoSphere(uint8_t divisions);
    EXPORT ~IcoSphere();
};
