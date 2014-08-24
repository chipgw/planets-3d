#ifndef PLANET_H
#define PLANET_H

#include <QVector>
#include <QVector3D>

class Planet {
private:
    float mass_p;
    float radius_p;

public:
    Planet(QVector3D p = QVector3D(), QVector3D v = QVector3D(), float m = 100.0f);

    QVector3D position;
    QVector3D velocity;

    QVector<QVector3D> path;
    static int pathLength;
    static float pathRecordDistance;

    void updatePath();

    inline float radius() const { return radius_p; }

    void setMass(const float &m);
    inline float mass() const { return mass_p; }
};

#endif // PLANET_H
