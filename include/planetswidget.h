#ifndef PlanetsWidget_H
#define PlanetsWidget_H

#include "camera.h"
#include "planetsuniverse.h"
#include "spheregenerator.h"
#include <QTime>
#include <QTimer>
#include <QMouseEvent>

#if QT_VERSION >= 0x050000
#include <QOpenGLFunctions>
#include <QOpenGLShader>
#else
#include <QGLFunctions>
#include <QGLShader>

#define QOpenGLShader               QGLShader
#define QOpenGLShaderProgram        QGLShaderProgram
#define QOpenGLFunctions            QGLFunctions
#define initializeOpenGLFunctions   initializeGLFunctions
#endif

#include <QGLWidget>

class PlanetsWidget : public QGLWidget, public QOpenGLFunctions {
    Q_OBJECT
private:
    enum PlacingStep{
        None,
        FreePositionXY,
        FreePositionZ,
        FreeVelocity,
        Firing
    };

    QOpenGLShaderProgram shaderTexture;
    QOpenGLShaderProgram shaderColor;
    GLuint texture;

    Camera camera;
    bool doScreenshot;

    int framecount;

    QTimer timer;
    QTime frameTime;
    QTime totalTime;

    QPoint lastmousepos;

    PlacingStep placingStep;
    Planet placing;
    QMatrix4x4 placingRotation;

    const static Sphere<128, 64> highResSphere;
    const static Sphere<32,  16> lowResSphere;

public:
    enum DisplaySettings{
        PlanetTrails  = (1<<0),
        SolidLineGrid = (1<<1),
        PointGrid     = (1<<2),
        PlanetColors  = (1<<3)
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

    float firingSpeed;
    float firingMass;

    int refreshRate;
    short displaysettings;

    const static QColor gridColor;
    unsigned int gridRange;
    QVector<QVector2D> gridPoints;

signals:
    void updatePlanetCountStatusMessage(const QString &text);
    void updateFPSStatusMessage(const QString &text);
    void updateAverageFPSStatusMessage(const QString &text);

public slots:
    void beginInteractiveCreation();
    void enableFiringMode(bool enable);
    void takeScreenshot();

protected:
    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();
    void mouseMoveEvent(QMouseEvent* e);
    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void mouseDoubleClickEvent(QMouseEvent* e);
    void wheelEvent(QWheelEvent* e);

    void drawPlanet(const Planet &planet);
    void drawPlanetPath(const Planet &planet);
    void drawPlanetColor(const Planet &planet, const QRgb &color);
    void drawPlanetWireframe(const Planet &planet, const QRgb &color = 0xff00ff00);
};

#endif // PlanetsWidget_H
