#ifndef CAMERA_H
#define CAMERA_H

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

struct Ray {
    glm::vec3 origin, direction;
};

class Camera {
public:
    Camera();

    glm::vec3 position;
    float distance;
    float xrotation;
    float zrotation;

    glm::mat4 projection;
    glm::mat4 camera;

    void bound();
    void reset();
    const glm::mat4 &setup();

    Ray getRay(const int &posX, const int &posY, const int &windowW, const int &windowH, bool normalize, float startDepth = 0.0f, float endDepth = 1.0f);
};

#endif // CAMERA_H
