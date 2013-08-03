#ifndef SPHEREGENERATOR_H
#define SPHEREGENERATOR_H

#include <QVector>

class Sphere{
public:
    QVector<QVector3D> verts;
    QVector<QVector2D> uv;
    QVector<unsigned int> triangles;
    QVector<unsigned int> lines;

    Sphere(unsigned int slices, unsigned int stacks);
};

#endif // SPHEREGENERATOR_H
