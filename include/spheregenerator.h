#ifndef SPHEREGENERATOR_H
#define SPHEREGENERATOR_H

#include "common.h"
#include <QVector2D>

class Sphere{
public:
    std::vector<QVector3D> verts;
    std::vector<QVector2D> uv;
    std::vector<unsigned int> triangles;
    std::vector<unsigned int> lines;

    Sphere(unsigned int slices, unsigned int stacks);
};

#endif // SPHEREGENERATOR_H
