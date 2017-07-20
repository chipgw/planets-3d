#pragma once

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

    Circle(uint32_t slices);
    ~Circle();
};

class Sphere {
public:
    const uint32_t vertexCount;
    const uint32_t triangleCount;
    const uint32_t lineCount;

    Vertex* verts;
    uint32_t* triangles;
    uint32_t* lines;

    Sphere(uint32_t slices, uint32_t stacks);
    ~Sphere();
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

    IcoSphere(uint8_t divisions);
    ~IcoSphere();
};
