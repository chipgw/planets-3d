#include "planet.h"
#include <qmath.h>

Planet::Planet(QVector3D p, QVector3D v, float m) : position(p), velocity(v) {
    setMass(m);
}

void Planet::updatePath(){
    if(path.isEmpty() || (path.back() - this->position).lengthSquared() > pathRecordDistance){
        path.append(this->position);
    }

    // doing this even if it hasn't been recorded allows it to get shorter.
    if(path.size() > pathLength){
        path.remove(0);
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

unsigned int Planet::pathLength = 200;

float Planet::pathRecordDistance = 0.1f;
