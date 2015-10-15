#include "include/grid.h"
#include <glm/gtx/norm.hpp>
#include <camera.h>

/* Returns the highest bit of an unsigned int.
 * Used to find the largest power of two less than the provided value. */
uint32_t highBit(uint32_t n) {
    n |= (n >>  1);
    n |= (n >>  2);
    n |= (n >>  4);
    n |= (n >>  8);
    n |= (n >> 16);
    return n - (n >> 1);
}

Grid::Grid() : range(32), color(0.8f, 1.0f, 1.0f, 0.4f), draw(false) { /* Nuthin. */ }

void Grid::update(const Camera& camera) {
    /* The resulting amount of vertecies is (range + 1) * 4
     * because there are 4 vertecies for every range value, plus for center lines at 0.  */
    if (points.size() != (range + 1) * 4) {
        points.clear();

        float bounds = range / 2.0f;
        for (float i = -bounds; i <= bounds; ++i) {
            points.push_back(glm::vec2(i, -bounds));
            points.push_back(glm::vec2(i,  bounds));

            points.push_back(glm::vec2(-bounds, i));
            points.push_back(glm::vec2( bounds, i));
        }
    }

    /* Use the camera matrix to approximate the distance from the universe center. */
    float distance = glm::pow(glm::length(camera.position) + camera.distance, 2.0/3.0);
    scale = float(highBit(uint32_t(distance)));
    alphafac = distance / scale - 1.0f;
}
