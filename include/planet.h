#ifndef PLANET_H
#define PLANET_H

#include "common.h"

class Planet {
private:
    float mass_p;
    float radius_p;

public:
    Planet(QVector3D p = QVector3D(), QVector3D v = QVector3D(), float m = 100.0f);

    QVector3D position;
    QVector3D velocity;

    std::vector<QVector3D> path;
    static unsigned int pathLength;

    void updatePath();

    float radius() const;

    float setMass(float m);
    float mass() const;
};

#endif // PLANET_H
