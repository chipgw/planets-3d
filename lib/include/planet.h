#pragma once

#include "types.h"
#include <vector>
#include <glm/vec3.hpp>

class Planet {
private:
    float mass_p;
    float radius_p;

public:
    EXPORT Planet(glm::vec3 p = glm::vec3(), glm::vec3 v = glm::vec3(), float m = 100.0f);

    glm::vec3 position;
    glm::vec3 velocity;

    std::vector<glm::vec3> path;

    void updatePath(size_t pathLength, float pathRecordDistance);

    inline float radius() const { return radius_p; }

    EXPORT void setMass(const float &m);
    inline float mass() const { return mass_p; }
};
