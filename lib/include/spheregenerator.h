#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec2 uv;
    float padding;
};

template <uint32_t slices> class Circle {
public:
    static const uint32_t vertexCount = slices;
    static const uint32_t lineCount = slices * 2;

    Vertex verts[vertexCount];
    uint32_t lines[lineCount];

    Circle();
};

template <uint32_t slices> Circle<slices>::Circle() {
    float step = (2.0f * glm::pi<float>()) / slices;

    uint32_t currentLine = 0;

    for (uint32_t current = 0; current < slices; ++current) {
        verts[current].position[0] = glm::cos(current * step);
        verts[current].position[1] = glm::sin(current * step);
        verts[current].position[2] = 0.0f;

        if (current == (slices - 1)) {
            lines[currentLine++] = current;
            lines[currentLine++] = 0;
        }else{
            lines[currentLine++] = current;
            lines[currentLine++] = current + 1;
        }
    }
}

template <uint32_t slices, uint32_t stacks> class Sphere {
public:
    static const uint32_t vertexCount = (slices + 1) * (stacks + 1);
    static const uint32_t triangleCount = slices * stacks * 6;
    static const uint32_t lineCount = slices * stacks * 4;

    Vertex verts[vertexCount];
    uint32_t triangles[triangleCount];
    uint32_t lines[lineCount];

    Sphere(bool flipV = false);
};

template <uint32_t slices, uint32_t stacks> Sphere<slices, stacks>::Sphere(bool flipV) {
    float vstep = glm::pi<float>() / stacks;
    float hstep = (2.0f * glm::pi<float>()) / slices;

    uint32_t currentTriangle = 0;
    uint32_t currentLine = 0;

    for (uint32_t v = 0; v <= stacks; ++v) {
        float z = glm::cos(v * vstep);
        float r = glm::sin(v * vstep);

        for (uint32_t h = 0; h <= slices; ++h) {
            uint32_t w = slices + 1;
            uint32_t current = v * w + h;

            verts[current].position.x = glm::cos(h * hstep) * r;
            verts[current].position.y = glm::sin(h * hstep) * r;
            verts[current].position.z = z;

            verts[current].normal = verts[current].position;

            verts[current].uv.x = float(h) / float(slices);
            verts[current].uv.y = flipV ? 1.0f - float(v) / float(stacks) : float(v) / float(stacks);

            if (h != slices && v != stacks) {
                triangles[currentTriangle++] = current;
                triangles[currentTriangle++] = current + w;
                triangles[currentTriangle++] = current + 1;

                triangles[currentTriangle++] = current + w + 1;
                triangles[currentTriangle++] = current + 1;
                triangles[currentTriangle++] = current + w;


                lines[currentLine++] = current;
                lines[currentLine++] = current + w;

                lines[currentLine++] = current;
                lines[currentLine++] = current + 1;
            }
        }
    }
}
