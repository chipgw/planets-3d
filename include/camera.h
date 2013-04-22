#ifndef CAMERA_H
#define CAMERA_H

#include "common.h"

class Camera {
public:
    Camera();

    glm::vec3 position;
    float distance;
    float xrotation;
    float zrotation;

    glm::mat4 projection;
    glm::mat4 camera;

    void setup();
};

#endif // CAMERA_H
