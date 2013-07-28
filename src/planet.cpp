#include "planet.h"

Planet::Planet(QVector3D p, QVector3D v, float m) : position(p), velocity(v) {
    setMass(m);
}

void Planet::updatePath(){
    if(path.size() < 1 || (path.back() - this->position).length() > 0.05f){
        path.push_back(this->position);
    }

    // doing this even if it hasn't been recorded allows it to get shorter.
    if(path.size() > pathLength){
        path.erase(path.begin());
    }
}

float Planet::radius() const {
    return radius_p;
}

float Planet::setMass(float m){
    mass_p = m;

    radius_p = pow((3.0f * m / 4.0f) * M_PI, 1.0f / 3.0f) * 0.1f;
}

float Planet::mass() const {
    return mass_p;
}

unsigned int Planet::pathLength = 200;
