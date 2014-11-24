#ifndef GRID_H
#define GRID_H

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

    void update(const glm::mat4 &camera);
    void toggle() { draw = !draw; }
};

#endif // GRID_H
