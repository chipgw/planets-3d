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

void Grid::update(const Camera& camera) {
    /* The resulting amount of vertecies is (range + 1) * 4
     * because there are 4 vertecies for every range value, plus for center lines at 0.  */
    if (points.size() != (range * 2 + 1) * 4) {
        points.clear();

        float bounds = range;
        for (float i = -bounds; i <= bounds; ++i) {
            points.push_back(glm::vec2(i, -bounds));
            points.push_back(glm::vec2(i,  bounds));

            points.push_back(glm::vec2(-bounds, i));
            points.push_back(glm::vec2( bounds, i));
        }
    }

    /* Use the camera distance & position to calculate the grid size. */
    float distance = glm::pow(glm::length(camera.position) + camera.distance, 2.0f/3.0f);

    /* The scale is distance rounded to the nearest power of two. */
    scale = float(highBit(uint32_t(distance)));

    /* How much of a difference is there between the distance value and the scale value? */
    alphafac = distance / scale - 1.0f;
}
