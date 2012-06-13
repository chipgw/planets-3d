#include "camera.h"

Camera::Camera() : location(0,0,0) {
    distance = 10;
    xrotation = 45;
    zrotation = 0;
}

void Camera::setup(){
    glTranslatef(0,0,-distance);
    glRotatef(xrotation-90,1,0,0);
    glRotatef(zrotation,0,0,1);
    glTranslatef(-location.x,-location.y,-location.z);
}
