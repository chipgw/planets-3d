#include "camera.h"
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera() : position(0.0f) {
    distance = 10.0f;
    xrotation = 45.0f;
    zrotation = 0.0f;
}

void Camera::setup(){
    camera = glm::translate(projection, glm::vec3(0.0f, 0.0f, -distance));
    camera = glm::rotate(camera, xrotation - 90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
    camera = glm::rotate(camera, zrotation, glm::vec3(0.0f, 0.0f, 1.0f));
    camera = glm::translate(camera, -position);
}
