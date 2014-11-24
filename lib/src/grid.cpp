#include "include/grid.h"
#include <glm/gtx/norm.hpp>

int highBit(unsigned int n) {
    n |= (n >>  1);
    n |= (n >>  2);
    n |= (n >>  4);
    n |= (n >>  8);
    n |= (n >> 16);
    return n - (n >> 1);
}

Grid::Grid() : range(32), color(0.8f, 1.0f, 1.0f, 0.4f), draw(false){ /* Nuthin. */ }

void Grid::update(const glm::mat4& camera){
    if(points.size() != (range + 1) * 8){
        points.clear();

        float bounds = range / 2.0f;
        for(float i = -bounds; i <= bounds; ++i){
            points.push_back(glm::vec2(i, -bounds));
            points.push_back(glm::vec2(i,  bounds));

            points.push_back(glm::vec2(-bounds, i));
            points.push_back(glm::vec2( bounds, i));
        }
    }

    float distance = std::cbrt(glm::length2(camera[3]));
    scale = highBit(distance);
    alphafac = distance / scale - 1.0f;
}
