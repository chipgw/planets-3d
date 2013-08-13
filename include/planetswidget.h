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
        PointGrid     = (1<<3),
        PlanetColors  = (1<<4)
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

    PlanetsUniverse universe;

    QRgb following;
    FollowingState followState;
    GLuint texture;

    QOpenGLShaderProgram shaderTexture;
    QOpenGLShaderProgram shaderColor;

    Camera camera;
    bool doScreenshot;

    PlacingStep placingStep;
    Planet placing;
    QMatrix4x4 placingRotation;

    int framecount;

    QTimer timer;
    QTime frameTime;
    QTime totalTime;

    int refreshRate;

    QPoint lastmousepos;

    short displaysettings;

    const static QColor gridColor;
    int gridRange;
    QVector<QVector2D> gridPoints;

    const static Sphere highResSphere;
    const static Sphere lowResSphere;

    void drawPlanet(const Planet &planet);
    void drawPlanetPath(const Planet &planet);
    void drawPlanetColor(const Planet &planet, const QRgb &color);
    void drawPlanetWireframe(const Planet &planet, const QRgb &color = 0xff00ff00);

signals:
    void updatePlanetCountStatusMessage(const QString &text);
    void updateFPSStatusMessage(const QString &text);
    void updateAverageFPSStatusMessage(const QString &text);

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
