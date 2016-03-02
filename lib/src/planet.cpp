#include "planet.h"
#include <glm/gtx/norm.hpp>

Planet::Planet(glm::vec3 p, glm::vec3 v, float m) : position(p), velocity(v) {
    setMass(m);
}

void Planet::updatePath(size_t pathLength, float pathRecordDistance) {
    /* If we have gone far enough, add a new point to the path. */
    if (path.size() < 2 || glm::distance2(path[path.size() - 2], position) > pathRecordDistance)
        path.push_back(position);
    else
        /* Otherwise update the last element to the current position. */
        path.back() = position;

    /* Delete any elements beyond the path size limit. */
    if (path.size() > pathLength)
        path.erase(path.begin(), path.end() - pathLength);
}

void Planet::setMass(const float& m) {
    mass_p = m;

    /* Don't bother calculating the radius if the mass is zero or less. */
    radius_p = m <= 0.0f ? 0.0f : std::cbrt((3.0f * m / 4.0f) * glm::pi<float>());
}
