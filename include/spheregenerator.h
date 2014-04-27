#ifndef SPHEREGENERATOR_H
#define SPHEREGENERATOR_H

#include <QVector2D>
#include <QVector3D>
#include <qmath.h>

template <unsigned int slices> class Circle {
public:
    static const unsigned int vertexCount = slices;
    static const unsigned int lineCount = slices * 2;

    QVector3D verts[vertexCount];
    unsigned int lines[lineCount];

    Circle();
};

template <unsigned int slices> Circle<slices>::Circle(){
    float step = float(2.0 * M_PI) / slices;

    unsigned int currentLine = 0;

    for(unsigned int current = 0; current < slices; current++){
        verts[current] = QVector3D(cos(current * step), sin(current * step), 0.0f);

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

    QVector3D verts[vertexCount];
    QVector2D uv[vertexCount];
    unsigned int triangles[triangleCount];
    unsigned int lines[lineCount];

    Sphere();
};

template <unsigned int slices, unsigned int stacks> Sphere<slices, stacks>::Sphere(){
    float vstep = float(M_PI) / stacks;
    float hstep = float(2.0 * M_PI) / slices;

    unsigned int currentTriangle = 0;
    unsigned int currentLine = 0;

    for(int v = 0; v <= stacks; v++){
        float z = cos(v * vstep);
        float r = sin(v * vstep);

        for(int h = 0; h <= slices; h++){
            unsigned int w = slices + 1;
            unsigned int current = v * w + h;

            verts[current] = QVector3D(cos(h * hstep) * r, sin(h * hstep) * r, z);

            uv[current] = QVector2D(float(h) / float(slices), 1.0f - float(v) / float(stacks));

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
