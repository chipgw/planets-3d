#ifndef PlanetsWidget_H
#define PlanetsWidget_H

#include "camera.h"
#include "planetsuniverse.h"
#include "spheregenerator.h"
#include <QTime>
#include <QTimer>
#include <QDir>

class QMouseEvent;

#if QT_VERSION >= 0x050000
#include <QOpenGLFunctions>
#include <QOpenGLShader>

#if QT_VERSION >= 0x050200
class QOpenGLTexture;
#define PLANETS3D_USE_QOPENGLTEXTURE
#endif
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
        NotPlacing,
        FreePositionXY,
        FreePositionZ,
        FreeVelocity,
        Firing,
        OrbitalPlanet,
        OrbitalPlane
    };

    QOpenGLShaderProgram shaderTexture;
    int shaderTexture_cameraMatrix, shaderTexture_modelMatrix;

    QOpenGLShaderProgram shaderColor;
    int shaderColor_cameraMatrix, shaderColor_modelMatrix, shaderColor_color;

    const static int vertex, uv;

#ifdef PLANETS3D_USE_QOPENGLTEXTURE
    QOpenGLTexture *texture;
#endif

    Camera camera;
    bool doScreenshot;

    int framecount;
    int refreshRate;
    QTimer timer;
    QTime frameTime;
    QTime totalTime;

    QPoint lastmousepos;

    PlacingStep placingStep;
    Planet placing;
    QMatrix4x4 placingRotation;
    float placingOrbitalRadius;

    const static Sphere<64, 32> highResSphere;
    const static Sphere<32, 16> lowResSphere;
    const static Circle<64> circle;

    const static QColor trailColor;
    const static QColor gridColor;
    unsigned int gridRange;
    QVector<QVector2D> gridPoints;

public:
    enum FollowingState{
        FollowNone,
        Single,
        PlainAverage,
        WeightedAverage
    };

    PlanetsWidget(QWidget *parent = 0);
    ~PlanetsWidget();

    PlanetsUniverse universe;

    QRgb following;
    FollowingState followState;

    float firingSpeed;
    float firingMass;
    float drawScale;
    bool drawGrid;
    bool drawPlanetTrails;
    bool drawPlanetColors;
    bool hidePlanets;

    QDir screenshotDir;

signals:
    void updateFPSStatusMessage(const QString &text);
    void updateAverageFPSStatusMessage(const QString &text);

public slots:
    void beginInteractiveCreation();
    void enableFiringMode(bool enable);
    void beginOrbitalCreation();
    void takeScreenshot();
    void updateGrid();
    void setGridRange(int value);
    void followNext();
    void followPrevious();

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
    void drawPlanetColor(const Planet &planet, const QRgb &color);
    void drawPlanetWireframe(const Planet &planet, const QRgb &color = 0xff00ff00);
};

#endif // PlanetsWidget_H
