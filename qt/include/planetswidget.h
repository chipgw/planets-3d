#pragma once

#include "placinginterface.h"
#include "planetsuniverse.h"
#include "spheregenerator.h"
#include "grid.h"
#include "camera.h"
#include <QElapsedTimer>
#include <QTimer>
#include <QDir>

class QMouseEvent;

#include <QOpenGLFunctions>
#include <QOpenGLShader>
#include <QOpenGLTexture>
#include <QOpenGLWidget>
#include <QOpenGLBuffer>

#ifdef PLANETS3D_QT_USE_SDL_GAMEPAD
#include "sdlgamepad.h"
#endif

#define NUM_PLANET_TEXTURES 7

class PlanetsWidget : public QOpenGLWidget, public QOpenGLFunctions {
    Q_OBJECT
private:
    /* GL shader and uniform handles for texture shader. */
    QOpenGLShaderProgram shaderTexture;
    int shaderTexture_cameraMatrix, shaderTexture_viewMatrix, shaderTexture_modelMatrix, shaderTexture_lightDir;

    /* GL shader and uniform handles for color shader. */
    QOpenGLShaderProgram shaderColor;
    int shaderColor_cameraMatrix, shaderColor_modelMatrix, shaderColor_color;

    QOpenGLTexture* textures_diff[NUM_PLANET_TEXTURES];
    QOpenGLTexture* textures_nrm[NUM_PLANET_TEXTURES];

    Camera camera;

    /* Total amount of frames drawn since the creation of the widget. */
    int frameCount = 0;
    /* The desired refresh rate, in milliseconds. */
    int refreshRate;

    /* Time since the start of the frame. */
    QElapsedTimer frameTime;
    /* Total amount of time the widget has been running. */
    QElapsedTimer totalTime;

    /* The position of the mouse cursor last mouse movement event. */
    QPoint lastMousePos;

    QOpenGLBuffer highResSphereVerts;
    QOpenGLBuffer highResSphereTris;
    unsigned int highResSphereTriCount;

    QOpenGLBuffer lowResSphereVerts;
    QOpenGLBuffer lowResSphereLines;
    unsigned int lowResSphereLineCount;

    QOpenGLBuffer circleVerts;
    QOpenGLBuffer circleLines;
    unsigned int circleLineCount;

    QOpenGLBuffer gridBuffer;

    const static QColor trailColor;

#ifdef PLANETS3D_QT_USE_SDL_GAMEPAD
    PlanetsGamepad gamepad;
#endif

public:
    PlanetsWidget(QWidget *parent = nullptr);

    PlanetsUniverse universe;

    PlacingInterface placing;

    Grid grid;

    /* Scale to draw the planets at. 1.0 is the default scale. */
    float drawScale = 1.0f;
    /* Do we draw trails? */
    bool drawPlanetTrails = false;
    /* Draw a circle for each planet on the XY plane, along with a connecting line. */
    bool drawPlanarCircles = false;
    /* Do we hide the textured planet spheres? */
    bool hidePlanets = false;

    /* Where we save screenshots to. */
    QDir screenshotDir;

signals:
    /* Update the statusbar messages. */
    void updateFPSStatusMessage(const QString& text);
    void updateAverageFPSStatusMessage(const QString& text);
    void statusBarMessage(const QString& text, int timeout = 0);

public slots:
    /* Slots for placing functions. */
    void beginInteractiveCreation() { placing.beginInteractiveCreation(); }
    void enableFiringMode(bool enable) { placing.enableFiringMode(enable); }
    void beginOrbitalCreation() { placing.beginOrbitalCreation(); }

    void takeScreenshot();

    void setGridRange(int value) { grid.range = value; }

    /* Slots for camera functions. */
    void followNext() { camera.followNext(); }
    void followPrevious() { camera.followPrevious(); }
    void followSelection() { camera.followSelection(); }
    void clearFollow() { camera.clearFollow(); camera.position = glm::vec3(); }
    void followPlainAverage() { camera.followPlainAverage(); }
    void followWeightedAverage() { camera.followWeightedAverage(); }

protected:
    /* Overriden from GLWidget. */
    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();

    void render();

    /* Mouse event functions, these pass data to the PlacingInterface. */
    void mouseMoveEvent(QMouseEvent* e);
    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent*);
    void mouseDoubleClickEvent(QMouseEvent* e);
    void wheelEvent(QWheelEvent* e);

    /* Draw a wireframe planet. Expects low res sphere VBO & IBO to be bound. */
    void drawPlanetWireframe(const Planet& planet, const QColor& color = 0xff00ff00);
};
