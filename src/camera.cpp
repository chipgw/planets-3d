#include "camera.h"

Camera::Camera() {
    reset();
}

void Camera::reset(){
    distance = 100.0f;
    xrotation = 45.0f;
    zrotation = 0.0f;
}

const QMatrix4x4 &Camera::setup(){
    camera = projection;
    camera.translate(0.0f, 0.0f, -distance);
    camera.rotate(xrotation - 90.0f, QVector3D(1.0f, 0.0f, 0.0f));
    camera.rotate(zrotation, QVector3D(0.0f, 0.0f, 1.0f));
    camera.translate(-position);
    return camera;
}
