#ifndef PlanetsWidget_H
#define PlanetsWidget_H

#include "common.h"
#include "planet.h"
#include "camera.h"
#include <QTime>
#include <QTimer>
#include <QMouseEvent>

class PlanetsWidget : public QGLWidget {
    Q_OBJECT
public:
    enum GridDisplayMode{
        Off,
        SolidLine,
        Points
    };

    PlanetsWidget(QWidget *parent = 0);
    ~PlanetsWidget();

    QList<Planet*> planets;
    Planet* selected;
    GLuint texture;

    Camera camera;
    bool doScreenshot;

    short placingStep;
    Planet placing;
    float placingXrotation,placingZrotation;

    float simspeed;
    int delay;
    int framecount;

    QTimer* timer;
    QTime frameTime;
    QTime totalTime;

    int framerate;

    QPoint lastmousepos;

    bool drawPaths;

    GridDisplayMode gridMode;
    glm::vec4 gridColor;
    int gridRange;
    void drawGrid();

    Planet* createPlanet(glm::vec3 position,glm::vec3 velocity,float mass);
    void deleteAll();
    void centerAll();

    void beginInteractiveCreation();

    bool save(const QString& filename);
    bool load(const QString& filename);

protected:
    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();
    void mouseMoveEvent(QMouseEvent* e);
    void mousePressEvent(QMouseEvent* e);
    void mouseDoubleClickEvent(QMouseEvent* e);
    void wheelEvent(QWheelEvent* e);
};

#endif // PlanetsWidget_H
