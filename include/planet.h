#ifndef PLANET_H
#define PLANET_H

#include "common.h"

class Planet {
public:
    Planet();
    ~Planet();

    float mass;

    QVector3D position;
    QVector3D velocity;

    std::vector<QVector3D> path;
    static unsigned int pathLength;

    void updatePath();
    float getRadius();

    bool operator ==(const Planet &p2);
};

#endif // PLANET_H
