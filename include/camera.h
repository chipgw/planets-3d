#ifndef CAMERA_H
#define CAMERA_H

#include "common.h"
#include <QVector3D>
#include <QMatrix4x4>

class Camera {
public:
    Camera();

    QVector3D position;
    float distance;
    float xrotation;
    float zrotation;

    QMatrix4x4 projection;
    QMatrix4x4 camera;

    void reset();
    void setup();
};

#endif // CAMERA_H
