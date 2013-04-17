#ifndef SPHEREGENERATOR_H
#define SPHEREGENERATOR_H

#include <common.h>

class Sphere{
public:
    std::vector<glm::vec3> verts;
    std::vector<glm::vec2> uv;
    std::vector<unsigned int> triangles;
    std::vector<unsigned int> lines;

    Sphere(unsigned int slices, unsigned int stacks);
};

#endif // SPHEREGENERATOR_H
