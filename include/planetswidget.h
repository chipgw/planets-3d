#ifndef PlanetsWidget_H
#define PlanetsWidget_H

#include "camera.h"
#include "planetsuniverse.h"
#include "spheregenerator.h"
#include <QElapsedTimer>
#include <QTimer>
#include <QDir>

class QMouseEvent;

#if QT_VERSION >= 0x050000
#include <QOpenGLFunctions>
#include <QOpenGLShader>

#if QT_VERSION >= 0x050200 && !defined(PLANETS3D_DISABLE_QOPENGLTEXTURE)
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

    int frameCount;
    int refreshRate;
    QTimer timer;
    QElapsedTimer frameTime;
    QElapsedTimer totalTime;

    QPoint lastMousePos;

    PlacingStep placingStep;
    Planet placing;
    glm::mat4 placingRotation;
    float placingOrbitalRadius;

    const static Sphere<64, 32> highResSphere;
    const static Sphere<32, 16> lowResSphere;
    const static Circle<64> circle;

    const static QColor trailColor;
    const static QColor gridColor;
    int gridRange;
    std::vector<float> gridPoints;

public:
    PlanetsWidget(QWidget *parent = 0);
    ~PlanetsWidget();

    PlanetsUniverse universe;

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
    void updatePlanetCountMessage(const QString &text);

public slots:
    void beginInteractiveCreation();
    void enableFiringMode(bool enable);
    void beginOrbitalCreation();
    void takeScreenshot();
    void updateGrid();
    void setGridRange(int value);
    void followNext();
    void followPrevious();
    void followSelection();
    void clearFollow();
    void followPlainAverage();
    void followWeightedAverage();

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
    void drawPlanetColor(const Planet &planet, const QColor &color);
    void drawPlanetWireframe(const Planet &planet, const QColor &color = 0xff00ff00);
};

#endif // PlanetsWidget_H
