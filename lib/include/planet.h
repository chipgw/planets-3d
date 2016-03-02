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

    /* Add a point to the path if the planet is specified distance (in units squared) from the last point. */
    void updatePath(size_t pathLength, float pathRecordDistance);

    /* Automatically set based on planet mass. */
    inline float radius() const { return radius_p; }

    /* Set the planet's mass and update the radius. */
    EXPORT void setMass(const float& m);

    inline float mass() const { return mass_p; }
};
