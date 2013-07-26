#ifndef PLANET_H
#define PLANET_H

#include "common.h"

class Planet {
public:
    Planet(QVector3D p = QVector3D(), QVector3D v = QVector3D(), float m = 100.0f);

    float mass;

    QVector3D position;
    QVector3D velocity;

    std::vector<QVector3D> path;
    static unsigned int pathLength;

    void updatePath();
    float getRadius() const;
};

#endif // PLANET_H
