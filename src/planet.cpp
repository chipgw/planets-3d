#include "planet.h"

Planet::Planet(QVector3D p, QVector3D v, float m) : position(p), velocity(v), mass(m) {

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

float Planet::getRadius(){
    return pow(3.0f * mass / 4.0f * M_PI, 1.0f / 3.0f) * 0.1f;
}

unsigned int Planet::pathLength = 200;
