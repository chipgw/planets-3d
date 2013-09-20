#include "planet.h"
#include <qmath.h>

Planet::Planet(QVector3D p, QVector3D v, float m) : position(p), velocity(v) {
    setMass(m);
}

void Planet::updatePath(){
    if(path.size() < 1 || (path.back() - this->position).lengthSquared() > 5.0e-3f){
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

    radius_p = pow((3.0f * m / 4.0f) * M_PI, 1.0f / 3.0f) * 0.1f;
}

float Planet::mass() const {
    return mass_p;
}

unsigned int Planet::pathLength = 200;
