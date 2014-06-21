#include "planet.h"
#include <qmath.h>

Planet::Planet(QVector3D p, QVector3D v, float m) : position(p), velocity(v) {
    setMass(m);
}

void Planet::updatePath(){
    if(path.size() < 2 || (path[path.size() - 2] - position).lengthSquared() > pathRecordDistance){
        path.append(position);
    } else {
        path.back() = position;
    }

    if(path.size() > pathLength){
        path.remove(0, path.size() - pathLength);
    }
}

float Planet::radius() const {
    return radius_p;
}

void Planet::setMass(const float &m){
    mass_p = m;
    radius_p = 0.0f;

    if(m > 0.0f){
        radius_p = pow((3.0f * m / 4.0f) * M_PI, 1.0f / 3.0f);
    }
}

float Planet::mass() const {
    return mass_p;
}

int Planet::pathLength = 200;

float Planet::pathRecordDistance = 0.25f;
