#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "types.h"

class Grid {
public:
    bool draw = false;
    uint32_t range = 16;
    glm::vec4 color = glm::vec4(0.8f, 1.0f, 1.0f, 0.4f);

    std::vector<glm::vec2> points;

    /* Both of these variables are for the larger of the two grid displays */
    float scale;
    float alphafac;

    /* Update all contained generated data. Returns true if the points array was regenerated. */
    EXPORT bool update(const Camera& camera);

    /* Toggle drawing the grid. */
    inline void toggle() { draw = !draw; }
};
