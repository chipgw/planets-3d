#ifndef PlanetsWidget_H
#define PlanetsWidget_H

#include "common.h"
#include "planet.h"
#include "camera.h"
#include "spheregenerator.h"
#include <QTime>
#include <QTimer>
#include <QMouseEvent>
#include <QGLWidget>

class PlanetsWidget : public QGLWidget {
    Q_OBJECT
public:
    enum DisplaySettings{
        PlanetTrails  = (1<<0),
        MotionBlur    = (1<<1),
        SolidLineGrid = (1<<2),
        PointGrid     = (1<<3)
    };
    enum PlacingStep{
        None,
        FreePositionXY,
        FreePositionZ,
        FreeVelocity,
    };
    enum FollowingState{
        FollowNone,
        Single,
        PlainAverage,
        WeightedAverage
    };

    PlanetsWidget(QWidget *parent = 0);
    ~PlanetsWidget();

    QList<Planet> planets;
    Planet* selected;
    Planet* following;
    FollowingState followState;
    GLuint texture;

    Camera camera;
    bool doScreenshot;

    PlacingStep placingStep;
    Planet placing;
    glm::mat4 placingRotation;

    float simspeed;
    int stepsPerFrame;
    int delay;
    int framecount;

    QTimer* timer;
    QTime frameTime;
    QTime totalTime;

    int framerate;

    QPoint lastmousepos;

    short displaysettings;

    glm::vec4 gridColor;
    int gridRange;
    std::vector<glm::vec2> gridPoints;

    Sphere highResSphere;
    Sphere lowResSphere;

    Planet &createPlanet(glm::vec3 position, glm::vec3 velocity, float mass);
    void deleteAll();

    bool save(const QString& filename);
    bool load(const QString& filename);

    void drawPlanet(Planet &planet);
    void drawPlanetPath(Planet &planet);
    void drawPlanetBounds(Planet &planet, GLenum drawmode = GL_LINES, bool selectioncolor = false);

signals:
    void updateFPSStatusMessage(const QString &text);
    void updateAverageFPSStatusMessage(const QString &text);
    void updateSimspeedStatusMessage(const QString &text);

public slots:
    void centerAll();

    void beginInteractiveCreation();

protected:
    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();
    void mouseMoveEvent(QMouseEvent* e);
    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void mouseDoubleClickEvent(QMouseEvent* e);
    void wheelEvent(QWheelEvent* e);
};

#endif // PlanetsWidget_H
