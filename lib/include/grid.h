#pragma once

#include <vector>
#include <glm/glm.hpp>

class Grid {
public:
    Grid();

    bool draw;
    uint32_t range;
    std::vector<glm::vec2> points;
    glm::vec4 color;

    /* Both of these variables are for the larger of the two grid displays */
    float scale;
    float alphafac;

    /* Update all contained generated data. */
    void update(const glm::mat4 &camera);

    /* Toggle drawing the grid. */
    void toggle() { draw = !draw; }
};
