#ifndef CAMERA_H
#define CAMERA_H

#include <QVector3D>
#include <QMatrix4x4>

struct Ray {
    QVector3D origin, direction;
};

class Camera {
public:
    Camera();

    QVector3D position;
    float distance;
    float xrotation;
    float zrotation;

    QMatrix4x4 projection;
    QMatrix4x4 camera;

    void bound();
    void reset();
    const QMatrix4x4 &setup();

    Ray getRay(const QPoint &pos, const QSize &window, bool normalize, float startDepth = 0.0f, float endDepth = 1.0f);
};

#endif // CAMERA_H
