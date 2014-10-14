#ifndef PLANET_H
#define PLANET_H

#include <vector>
#include <glm/vec3.hpp>

class Planet {
private:
    float mass_p;
    float radius_p;

public:
    Planet(glm::vec3 p = glm::vec3(), glm::vec3 v = glm::vec3(), float m = 100.0f);

    glm::vec3 position;
    glm::vec3 velocity;

    std::vector<glm::vec3> path;
    static std::vector<glm::vec3>::size_type pathLength;
    static float pathRecordDistance;

    void updatePath();

    inline float radius() const { return radius_p; }

    void setMass(const float &m);
    inline float mass() const { return mass_p; }
};

#endif // PLANET_H
