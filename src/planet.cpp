#include "planet.h"

Planet::Planet() {
    mass = 100.0f;
}

Planet::~Planet(){

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

bool Planet::operator ==(const Planet &p2){
    return this == &p2;
}

unsigned int Planet::pathLength = 200;
