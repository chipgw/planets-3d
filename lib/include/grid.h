#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "platform.h"

class Camera;

class Grid {
public:
    EXPORT Grid();

    bool draw;
    uint32_t range;
    std::vector<glm::vec2> points;
    glm::vec4 color;

    /* Both of these variables are for the larger of the two grid displays */
    float scale;
    float alphafac;

    /* Update all contained generated data. */
    EXPORT void update(const Camera &camera);

    /* Toggle drawing the grid. */
    inline void toggle() { draw = !draw; }
};
