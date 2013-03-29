#include "camera.h"

Camera::Camera() : position(0,0,0) {
    distance = 10;
    xrotation = 45;
    zrotation = 0;
}

void Camera::setup(){
    glTranslatef(0,0,-distance);
    glRotatef(xrotation-90,1,0,0);
    glRotatef(zrotation,0,0,1);
    glTranslatef(-position.x,-position.y,-position.z);
}
