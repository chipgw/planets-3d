#include "planet.h"
#include <glm/gtx/norm.hpp>

Planet::Planet(glm::vec3 p, glm::vec3 v, float m) : position(p), velocity(v) {
    setMass(m);
}

void Planet::updatePath(size_t pathLength, float pathRecordDistance){
    if(path.size() < 2 || glm::distance2(path[path.size() - 2], position) > pathRecordDistance){
        path.push_back(position);
    } else {
        path.back() = position;
    }

    if(path.size() > pathLength){
        path.erase(path.begin(), path.end() - pathLength);
    }
}

void Planet::setMass(const float &m){
    mass_p = m;
    radius_p = 0.0f;

    if(m > 0.0f){
        radius_p = std::cbrt((3.0f * m / 4.0f) * glm::pi<float>());
    }
}
