#ifndef SPHEREGENERATOR_H
#define SPHEREGENERATOR_H

#include <glm/glm.hpp>
#include <glm/gtx/constants.hpp>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec2 uv;
    float padding;
};

template <unsigned int slices> class Circle {
public:
    static const unsigned int vertexCount = slices;
    static const unsigned int lineCount = slices * 2;

    Vertex verts[vertexCount];
    unsigned int lines[lineCount];

    Circle();
};

template <unsigned int slices> Circle<slices>::Circle(){
    float step = (2.0f * glm::pi<float>()) / slices;

    unsigned int currentLine = 0;

    for(unsigned int current = 0; current < slices; ++current){
        verts[current].position[0] = glm::cos(current * step);
        verts[current].position[1] = glm::sin(current * step);
        verts[current].position[2] = 0.0f;

        if(current == (slices - 1)){
            lines[currentLine++] = current;
            lines[currentLine++] = 0;
        }else{
            lines[currentLine++] = current;
            lines[currentLine++] = current + 1;
        }
    }
}

template <unsigned int slices, unsigned int stacks> class Sphere {
public:
    static const unsigned int vertexCount = (slices + 1) * (stacks + 1);
    static const unsigned int triangleCount = slices * stacks * 6;
    static const unsigned int lineCount = slices * stacks * 4;

    Vertex verts[vertexCount];
    unsigned int triangles[triangleCount];
    unsigned int lines[lineCount];

    Sphere(bool flipV = false);
};

template <unsigned int slices, unsigned int stacks> Sphere<slices, stacks>::Sphere(bool flipV){
    float vstep = glm::pi<float>() / stacks;
    float hstep = (2.0f * glm::pi<float>()) / slices;

    unsigned int currentTriangle = 0;
    unsigned int currentLine = 0;

    for(unsigned int v = 0; v <= stacks; ++v){
        float z = glm::cos(v * vstep);
        float r = glm::sin(v * vstep);

        for(unsigned int h = 0; h <= slices; ++h){
            unsigned int w = slices + 1;
            unsigned int current = v * w + h;

            verts[current].position.x = glm::cos(h * hstep) * r;
            verts[current].position.y = glm::sin(h * hstep) * r;
            verts[current].position.z = z;

            verts[current].normal = verts[current].position;

            verts[current].uv.x = float(h) / float(slices);
            verts[current].uv.y = flipV ? 1.0f - float(v) / float(stacks) : float(v) / float(stacks);

            if(h != slices && v != stacks){
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

#endif // SPHEREGENERATOR_H
