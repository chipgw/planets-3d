#include "camera.h"
#include <qmath.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera() {
    reset();
}

void Camera::bound(){
    distance = qBound(10.0f, distance, 1.0e4f);
    xrotation = qBound(-90.0f, xrotation, 90.0f);
    zrotation = fmod(zrotation, 360.0f);
}

void Camera::reset(){
    distance = 100.0f;
    xrotation = 45.0f;
    zrotation = 0.0f;
}

const glm::mat4 &Camera::setup(){
    camera = glm::translate(projection, glm::vec3(0.0f, 0.0f, -distance));
    camera = glm::rotate(camera, xrotation - 90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
    camera = glm::rotate(camera, zrotation, glm::vec3(0.0f, 0.0f, 1.0f));
    camera = glm::translate(camera, -position);
    return camera;
}

Ray Camera::getRay(const QPoint &pos, const QSize &window, bool normalize, float startDepth, float endDepth){
    Ray ray;

    glm::mat4 model;
    glm::vec3 windowCoord(pos.x(), window.height() - pos.y(), startDepth);
    glm::vec4 viewport(0.0f, 0.0f, window.width(), window.height());

    ray.origin = glm::unProject(windowCoord, model, camera, viewport);

    windowCoord.z = endDepth;

    ray.direction = ray.origin - glm::unProject(windowCoord, model, camera, viewport);

    if(normalize){
        ray.direction = glm::normalize(ray.direction);
    }

    return ray;
}
