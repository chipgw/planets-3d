#ifndef CAMERA_H
#define CAMERA_H

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
    const QMatrix4x4 &setup();
};

#endif // CAMERA_H
