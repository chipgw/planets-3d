#ifndef COMMON_H
#define COMMON_H

#include <Qt>
#include <QApplication>
#include <QDir>
#include <QDebug>
#include <QFile>
#include <QtOpenGL/QGLWidget>
#include <QMap>
#include <QMessageBox>
#include <math.h>
#include <vector>
#include <GL/glu.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_query.hpp>

// just in case math.h did not declare M_PI
#ifndef M_PI
#define M_PI 3.141592654
#endif

const float gravityconst = 6.67e-1f;

namespace version{
    extern const int major;
    extern const int minor;
    extern const QString git_revision;
    QString getVersionString();
    extern const QString build_type;
}

#endif // COMMON_H
