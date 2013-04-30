#ifndef PlanetsWidget_H
#define PlanetsWidget_H

#include "camera.h"
#include "planetsuniverse.h"
#include "spheregenerator.h"
#include <QTime>
#include <QTimer>
#include <QMouseEvent>
#include <QOpenGLFunctions>
#include <QOpenGLShader>
#include <QGLWidget>

class PlanetsWidget : public QGLWidget, public QOpenGLFunctions {
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

    static const GLuint vertexAttrib;
    static const GLuint uvAttrib;

    PlanetsWidget(QWidget *parent = 0);
    ~PlanetsWidget();

    PlanetsUniverse universe;

    Planet* following;
    FollowingState followState;
    GLuint texture;

    QOpenGLShaderProgram shaderTexture;
    QOpenGLShaderProgram shaderColor;

    Camera camera;
    bool doScreenshot;

    PlacingStep placingStep;
    Planet placing;
    QMatrix4x4 placingRotation;

    int stepsPerFrame;
    int delay;
    int framecount;

    QTimer* timer;
    QTime frameTime;
    QTime totalTime;

    int framerate;

    QPoint lastmousepos;

    short displaysettings;

    // not a QColor because that is byte based, this is float based.
    QVector4D gridColor;
    int gridRange;
    std::vector<QVector2D> gridPoints;

    Sphere highResSphere;
    Sphere lowResSphere;

    void drawPlanet(Planet &planet);
    void drawPlanetPath(Planet &planet);
    void drawPlanetBounds(Planet &planet, GLenum drawmode = GL_LINES, bool selectioncolor = false);

signals:
    void updateFPSStatusMessage(const QString &text);
    void updateAverageFPSStatusMessage(const QString &text);
    void updateSimspeedStatusMessage(const QString &text);

public slots:
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
