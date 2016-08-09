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

    glm::vec3 verts[vertexCount];
    uint32_t lines[lineCount];

    Circle();
};

template <uint32_t slices> Circle<slices>::Circle() {
    /* The amount of rotation needed for each slice. */
    float step = (2.0f * glm::pi<float>()) / slices;

    /* Keep track of the next index in the line index buffer. */
    uint32_t currentLine = 0;

    for (uint32_t current = 0; current < slices; ++current) {
        /* Calculate the coordinate of the vertex. */
        verts[current][0] = glm::cos(current * step);
        verts[current][1] = glm::sin(current * step);
        verts[current][2] = 0.0f;

        /* Add the current vertex to the index buffer. */
        lines[currentLine++] = current;
        /* If we're on the last slice, connect it to the first vertex, otherwise connect it to the next one. */
        lines[currentLine++] = current == (slices - 1) ? 0 : current + 1;
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

    Sphere();
};

template <uint32_t slices, uint32_t stacks> Sphere<slices, stacks>::Sphere() {
    /* The amount of rotation needed for each stack, ranging from pole to pole. */
    float vstep = glm::pi<float>() / stacks;
    /* The amount of rotation needed for each slice, ranging all the way around the sphere. */
    float hstep = (2.0f * glm::pi<float>()) / slices;

    /* Keep track of the next index in the line index buffer. */
    uint32_t currentTriangle = 0;
    uint32_t currentLine = 0;

    /* The offset for the index to connect to in the next stack.  */
    const uint32_t w = slices + 1;

    for (uint32_t v = 0; v <= stacks; ++v) {
        /* Calculate the height and radius of the stack. */
        float z = glm::cos(v * vstep);
        float r = glm::sin(v * vstep);

        for (uint32_t h = 0; h <= slices; ++h) {
            /* The index of the current vertex. */
            uint32_t current = v * w + h;

            /* Make a circle with the radiusof the current stack. */
            verts[current].position.x = glm::cos(h * hstep) * r;
            verts[current].position.y = glm::sin(h * hstep) * r;
            /* Well this is easy... */
            verts[current].position.z = z;

            /* The vertex normal is the same as the position, which is already normalized. */
            verts[current].normal = verts[current].position;

            /* Tangents are 90 degrees off from the circle coordinate with no z. */
            verts[current].tangent.x = -glm::sin(h * hstep);
            verts[current].tangent.y = glm::cos(h * hstep);
            verts[current].tangent.z = 0;

            /* The texture coordinate is simply how far along we are in our slizes/stacks. */
            verts[current].uv.x = float(h) / float(slices);
            verts[current].uv.y = float(v) / float(stacks);

            if (h != slices && v != stacks) {
                /* A triangle with the current vertex, the next one, and the one above it. */
                triangles[currentTriangle++] = current;
                triangles[currentTriangle++] = current + w;
                triangles[currentTriangle++] = current + 1;

                /* A triangle with the next vertex, the one above it, and the one above the current. */
                triangles[currentTriangle++] = current + w + 1;
                triangles[currentTriangle++] = current + 1;
                triangles[currentTriangle++] = current + w;

                /* A line connecting to the next vertex in the list. */
                lines[currentLine++] = current;
                lines[currentLine++] = current + 1;

                /* A line connecting to the vertex above this one. */
                lines[currentLine++] = current;
                lines[currentLine++] = current + w;
            }
        }
    }
}
