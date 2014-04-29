#ifndef VERSION_H
#define VERSION_H

#include <QString>

namespace version{
    extern const QString git_revision;
    extern const QString build_type;
    extern const QString compiler;
    extern const QString cmake_version;
}

#endif // VERSION_H
