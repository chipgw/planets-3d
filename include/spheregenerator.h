#ifndef SPHEREGENERATOR_H
#define SPHEREGENERATOR_H

#include <qmath.h>

struct Vertex {
    float position[3];
    float normal[3];
    float tangent[3];
    float uv[2];
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
    float step = float(2.0 * M_PI) / slices;

    unsigned int currentLine = 0;

    for(unsigned int current = 0; current < slices; ++current){
        verts[current].position[0] = cos(current * step);
        verts[current].position[1] = sin(current * step);
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

    Sphere();
};

template <unsigned int slices, unsigned int stacks> Sphere<slices, stacks>::Sphere(){
    float vstep = float(M_PI) / stacks;
    float hstep = float(2.0 * M_PI) / slices;

    unsigned int currentTriangle = 0;
    unsigned int currentLine = 0;

    for(unsigned int v = 0; v <= stacks; ++v){
        float z = cos(v * vstep);
        float r = sin(v * vstep);

        for(unsigned int h = 0; h <= slices; ++h){
            unsigned int w = slices + 1;
            unsigned int current = v * w + h;

            verts[current].position[0] = cos(h * hstep) * r;
            verts[current].position[1] = sin(h * hstep) * r;
            verts[current].position[2] = z;

            verts[current].normal[0] = verts[current].position[0];
            verts[current].normal[1] = verts[current].position[1];
            verts[current].normal[2] = verts[current].position[2];

            verts[current].uv[0] = float(h) / float(slices);
            verts[current].uv[1] = 1.0f - float(v) / float(stacks);

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
