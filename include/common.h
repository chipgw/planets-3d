#ifndef COMMON_H
#define COMMON_H

#include <QDebug>
#include <qmath.h>
#include <QVector3D>

// the gravity constant
const float gravityconst = 6.67e-11f;
// the factor for apparent velocity. (i.e. UI velocity * this = actual velocity, because it would be really really small if done right.)
const float velocityfac = 1.0e-4f;
// the maximum value of the simulation speed dial as a float.
const float speeddialmax = 10.0f;

namespace version{
    extern const int major;
    extern const int minor;
    extern const QString git_revision;
    QString getVersionString();
    extern const QString build_type;
}

#endif // COMMON_H
