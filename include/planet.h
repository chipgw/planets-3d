#ifndef PLANET_H
#define PLANET_H

#include "common.h"
#include "spheregenerator.h"
#include <QColor>

class Planet {
public:
    Planet();
    ~Planet();

    float mass;

    glm::vec3 position;
    glm::vec3 velocity;

    QColor selectionColor;
    static QColor nextSelectionColor;

    std::vector<glm::vec3> path;
    static unsigned int pathLength;

    static Sphere highResSphere;
    static Sphere lowResSphere;

    void draw();
    void drawPath();
    void updatePath();
    void drawBounds(GLenum drawmode = GL_LINES, bool selectioncolor = false);
    float getRadius();

    bool operator ==(const Planet &p2);
};

#endif // PLANET_H
