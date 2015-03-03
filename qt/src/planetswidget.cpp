#include "planetswidget.h"
#include <QDir>
#include <QMouseEvent>
#include <limits>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>

PlanetsWidget::PlanetsWidget(QWidget* parent) : QOpenGLWidget(parent),
    doScreenshot(false), frameCount(0), refreshRate(16), timer(this), placing(universe), drawScale(1.0f),
    camera(universe), drawPlanetTrails(false), drawPlanetColors(false), hidePlanets(false),
    screenshotDir(QDir::homePath() + "/Pictures/Planets3D-Screenshots/") {

    /* The screenshot directory I used to use. Someday I'll remove this movement code... */
    if(!screenshotDir.exists()){
        QDir oldDir = QDir::homePath() + "/Pictures/Planets3D Screenshots/";
        if(oldDir.exists()){
            oldDir.rename(oldDir.absolutePath(), screenshotDir.absolutePath());
        }else{
            screenshotDir.mkpath(screenshotDir.absolutePath());
        }
    }

#ifndef NDEBUG
    /* Don't cap framerate for debug builds. (Note: your GPU might do it anyway...) */
    refreshRate = 0;
#endif

    /* We want mouse movement events. */
    setMouseTracking(true);

    /* set up the timer for adding frames to the event queue. */
    timer.setSingleShot(true);
    connect(&timer, SIGNAL(timeout()), this, SLOT(update()));

    /* Don't let people make the widget really small. */
    setMinimumSize(QSize(100, 100));
}

void PlanetsWidget::initializeGL() {
    initializeOpenGLFunctions();

    /* Load the shaders from the qrc. */
    shaderTexture.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/texture.vsh");
    shaderTexture.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/texture.fsh");

    /* Bind the attribute handles. */
    shaderTexture.bindAttributeLocation("vertex", vertex);
    shaderTexture.bindAttributeLocation("uv", uv);

    shaderTexture.link();

    /* Get the uniform values */
    shaderTexture_cameraMatrix = shaderTexture.uniformLocation("cameraMatrix");
    shaderTexture_modelMatrix = shaderTexture.uniformLocation("modelMatrix");

    /* Load the shaders from the qrc. */
    shaderColor.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/color.vsh");
    shaderColor.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/color.fsh");

    /* Bind the attribute handle. */
    shaderColor.bindAttributeLocation("vertex", vertex);

    shaderColor.link();

    /* Get the uniform values */
    shaderColor_cameraMatrix = shaderColor.uniformLocation("cameraMatrix");
    shaderColor_modelMatrix = shaderColor.uniformLocation("modelMatrix");
    shaderColor_color = shaderColor.uniformLocation("color");

    /* Clear to black with depth of 1.0 */
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepthf(1.0f);

    /* Set and enable depth test function. */
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    /* Cull back faces. */
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    /* Enable alpha blending. */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* If the platform has GL_LINE_SMOOTH use it. */
#ifdef GL_LINE_SMOOTH
    glEnable(GL_LINE_SMOOTH);
#endif

    /* This vertex attribute should always be enabled. */
    shaderColor.enableAttributeArray(vertex);

    /* The one and only texture. */
    QImage img(":/textures/planet.png");

    texture = new QOpenGLTexture(img.mirrored());

    if(frameCount == 0){
        totalTime.start();
        frameTime.start();
    }
}

void PlanetsWidget::resizeGL(int width, int height) {
    glViewport(0, 0, width, height);

    camera.resizeViewport(width, height);
}

void PlanetsWidget::paintGL() {
    int delay = frameTime.nsecsElapsed() / 1000;
    frameTime.start();

    if(placing.step == PlacingInterface::NotPlacing || placing.step == PlacingInterface::Firing){
        universe.advance(delay);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    camera.setup();

    if(!hidePlanets){
        shaderTexture.bind();
        glUniformMatrix4fv(shaderTexture_cameraMatrix, 1, GL_FALSE, glm::value_ptr(camera.camera));
        shaderTexture.setUniformValue(shaderTexture_modelMatrix, QMatrix4x4());

        shaderTexture.enableAttributeArray(uv);

        texture->bind();

        for(const auto& i : universe){
            drawPlanet(i.second);
        }

        shaderTexture.disableAttributeArray(uv);
    }

    shaderColor.bind();
    glUniformMatrix4fv(shaderColor_cameraMatrix, 1, GL_FALSE, glm::value_ptr(camera.camera));

    if(drawPlanetColors){
        for(const auto& i : universe){
            drawPlanetWireframe(i.second, i.first);
        }
    }else if(!hidePlanets && universe.isSelectedValid()){
        drawPlanetWireframe(universe.getSelected());
    }

    if(drawPlanetTrails){
        shaderColor.setUniformValue(shaderColor_modelMatrix, QMatrix4x4());
        shaderColor.setUniformValue(shaderColor_color, trailColor);

        for(const auto& i : universe){
            shaderColor.setAttributeArray(vertex, GL_FLOAT, i.second.path.data(), 3);
            glDrawArrays(GL_LINE_STRIP, 0, GLsizei(i.second.path.size()));
        }
    }

    switch(placing.step){
    case PlacingInterface::FreeVelocity:{
        float length = glm::length(placing.planet.velocity) / PlanetsUniverse::velocityfac;

        if(length > 0.0f){
            glm::mat4 matrix = glm::translate(placing.planet.position);
            matrix = glm::scale(matrix, glm::vec3(placing.planet.radius()));
            matrix *= placing.rotation;
            glUniformMatrix4fv(shaderColor_modelMatrix, 1, GL_FALSE, glm::value_ptr(matrix));
            shaderColor.setUniformValue(shaderColor_color, trailColor);

            float verts[] = {  0.1f, 0.1f, 0.0f,
                               0.1f,-0.1f, 0.0f,
                              -0.1f,-0.1f, 0.0f,
                              -0.1f, 0.1f, 0.0f,

                               0.1f, 0.1f, length,
                               0.1f,-0.1f, length,
                              -0.1f,-0.1f, length,
                              -0.1f, 0.1f, length,

                               0.2f, 0.2f, length,
                               0.2f,-0.2f, length,
                              -0.2f,-0.2f, length,
                              -0.2f, 0.2f, length,

                               0.0f, 0.0f, length + 0.4f };

            static const GLubyte indexes[] = {  0,  1,  2,       2,  3,  0,

                                                1,  0,  5,       4,  5,  0,
                                                2,  1,  6,       5,  6,  1,
                                                3,  2,  7,       6,  7,  2,
                                                0,  3,  4,       7,  4,  3,

                                                5,  4,  9,       8,  9,  4,
                                                6,  5, 10,       9, 10,  5,
                                                7,  6, 11,      10, 11,  6,
                                                4,  7,  8,      11,  8,  7,

                                                9,  8, 12,
                                               10,  9, 12,
                                               11, 10, 12,
                                                8, 11, 12 };

            shaderColor.setAttributeArray(vertex, GL_FLOAT, verts, 3);
            glDrawElements(GL_TRIANGLES, sizeof(indexes), GL_UNSIGNED_BYTE, indexes);
        }
    }
    case PlacingInterface::FreePositionXY:
    case PlacingInterface::FreePositionZ:
        drawPlanetWireframe(placing.planet);
        break;
    case PlacingInterface::OrbitalPlane:
    case PlacingInterface::OrbitalPlanet:
        if(universe.isSelectedValid() && placing.orbitalRadius > 0.0f){
            glm::mat4 matrix = glm::translate(universe.getSelected().position);
            matrix = glm::scale(matrix, glm::vec3(placing.orbitalRadius));
            matrix *= placing.rotation;
            glUniformMatrix4fv(shaderColor_modelMatrix, 1, GL_FALSE, glm::value_ptr(matrix));
            shaderColor.setUniformValue(shaderColor_color, trailColor);

            shaderColor.setAttributeArray(vertex, GL_FLOAT, circle.verts, 3, sizeof(Vertex));
            glDrawElements(GL_LINES, circle.lineCount, GL_UNSIGNED_INT, circle.lines);

            drawPlanetWireframe(placing.planet);
        }
        break;
    default: break;
    }

    if(grid.draw){
        grid.update(camera.camera);

        /* The grid doesn't write to the depth buffer. */
        glDepthMask(GL_FALSE);

        shaderColor.setAttributeArray(vertex, GL_FLOAT, grid.points.data(), 2);

        glm::vec4 color = grid.color;
        color.a *= grid.alphafac;

        QMatrix4x4 matrix;
        matrix.scale(grid.scale);
        shaderColor.setUniformValue(shaderColor_modelMatrix, matrix);
        glUniform4fv(shaderColor_color, 1, glm::value_ptr(color));

        glDrawArrays(GL_LINES, 0, GLsizei(grid.points.size()));

        matrix.scale(0.5f);
        shaderColor.setUniformValue(shaderColor_modelMatrix, matrix);
        color.a = grid.color.a - color.a;
        glUniform4fv(shaderColor_color, 1, glm::value_ptr(color));

        glDrawArrays(GL_LINES, 0, GLsizei(grid.points.size()));

        glDepthMask(GL_TRUE);
    }

    if(doScreenshot){
        doScreenshot = false;

        QString filename = screenshotDir.absoluteFilePath("shot%1.png");
        int i = 0;
        while(QFile::exists(filename.arg(++i, 4, 10, QChar('0'))));
        filename = filename.arg(i, 4, 10, QChar('0'));

        QImage img = grabFramebuffer();
        if(!img.isNull() && img.save(filename)){
            qDebug("Screenshot saved to: %s", qPrintable(filename));
        }
    }

    timer.start(glm::max(0, refreshRate - int(frameTime.elapsed())));

    emit updateAverageFPSStatusMessage(tr("average fps: %1").arg(++frameCount * 1.0e3f / totalTime.elapsed()));
    emit updateFPSStatusMessage(tr("fps: %1").arg(1.0e6f / delay));

    if(universe.size() == 1){
        emit updatePlanetCountMessage(tr("1 planet"));
    }else{
        emit updatePlanetCountMessage(tr("%1 planets").arg(universe.size()));
    }
}

void PlanetsWidget::mouseMoveEvent(QMouseEvent* e){
    glm::ivec2 delta(lastMousePos.x() - e->x(), lastMousePos.y() - e->y());

    bool holdCursor = false;

    if(!placing.handleMouseMove(glm::ivec2(e->x(), e->y()), delta, camera, holdCursor)){
        if(e->buttons().testFlag(Qt::MiddleButton) || e->buttons().testFlag(Qt::RightButton)){
            if(e->modifiers().testFlag(Qt::ControlModifier)){
                camera.distance -= delta.y * camera.distance * 1.0e-2f;
                setCursor(Qt::SizeVerCursor);
                camera.bound();
            }else{
                camera.xrotation += delta.y * 0.01f;
                camera.zrotation += delta.x * 0.01f;

                camera.bound();

                holdCursor = true;
            }
        }
    }
    if(holdCursor){
        QCursor::setPos(mapToGlobal(lastMousePos));
        setCursor(Qt::BlankCursor);
    } else {
        lastMousePos = e->pos();
    }
}

void PlanetsWidget::mouseDoubleClickEvent(QMouseEvent* e){
    switch(e->button()){
    case Qt::LeftButton:
        /* Double clicking the left button while not placing sets or clears the planet currently being followed. */
        if(placing.step == PlacingInterface::NotPlacing){
            if(universe.isSelectedValid()){
                camera.followSelection();
            }else{
                camera.clearFollow();
                camera.position = glm::vec3();
            }
        }
        break;
    case Qt::MiddleButton:
    case Qt::RightButton:
        /* Double clicking the middle or right button resets the camera. */
        camera.reset();
        break;
    default: break;
    }
}

void PlanetsWidget::mousePressEvent(QMouseEvent* e){
    if(e->button() == Qt::LeftButton){
        glm::ivec2 pos(e->x(), e->y());
        if(!placing.handleMouseClick(pos, camera)){
            camera.selectUnder(pos);
        }
    }
}

void PlanetsWidget::mouseReleaseEvent(QMouseEvent *e){
    setCursor(Qt::ArrowCursor);
}

void PlanetsWidget::wheelEvent(QWheelEvent* e){
    if(!placing.handleMouseWheel(e->delta() * 1.0e-3f)){
        camera.distance -= e->delta() * camera.distance * 5.0e-4f;

        camera.bound();
    }
}

void PlanetsWidget::drawPlanet(const Planet &planet){
    glm::mat4 matrix = glm::translate(planet.position);
    matrix = glm::scale(matrix, glm::vec3(planet.radius() * drawScale));
    glUniformMatrix4fv(shaderTexture_modelMatrix, 1, GL_FALSE, glm::value_ptr(matrix));

    shaderTexture.setAttributeArray(vertex, GL_FLOAT, glm::value_ptr(highResSphere.verts[0].position), 3, sizeof(Vertex));
    shaderTexture.setAttributeArray(uv, GL_FLOAT, glm::value_ptr(highResSphere.verts[0].uv), 2, sizeof(Vertex));
    glDrawElements(GL_TRIANGLES, highResSphere.triangleCount, GL_UNSIGNED_INT, highResSphere.triangles);
}

void PlanetsWidget::drawPlanetColor(const Planet &planet, const QColor &color){
    shaderColor.setUniformValue(shaderColor_color, color);

    glm::mat4 matrix = glm::translate(planet.position);
    matrix = glm::scale(matrix, glm::vec3(planet.radius() * drawScale * 1.05f));
    glUniformMatrix4fv(shaderColor_modelMatrix, 1, GL_FALSE, glm::value_ptr(matrix));

    shaderColor.setAttributeArray(vertex, GL_FLOAT, glm::value_ptr(lowResSphere.verts[0].position), 3, sizeof(Vertex));
    glDrawElements(GL_TRIANGLES, lowResSphere.triangleCount, GL_UNSIGNED_INT, lowResSphere.triangles);
}

void PlanetsWidget::drawPlanetWireframe(const Planet &planet, const QColor &color){
    shaderColor.setUniformValue(shaderColor_color, color);

    glm::mat4 matrix = glm::translate(planet.position);
    matrix = glm::scale(matrix, glm::vec3(planet.radius() * drawScale * 1.05f));
    glUniformMatrix4fv(shaderColor_modelMatrix, 1, GL_FALSE, glm::value_ptr(matrix));

    shaderColor.setAttributeArray(vertex, GL_FLOAT, glm::value_ptr(lowResSphere.verts[0].position), 3, sizeof(Vertex));
    glDrawElements(GL_LINES, lowResSphere.lineCount, GL_UNSIGNED_INT, lowResSphere.lines);
}

const QColor PlanetsWidget::trailColor = QColor(0xcc, 0xff, 0xff, 0xff);

const Sphere<64, 32> PlanetsWidget::highResSphere = Sphere<64, 32>(true);
const Sphere<32, 16> PlanetsWidget::lowResSphere  = Sphere<32, 16>(true);

const Circle<64> PlanetsWidget::circle = Circle<64>();

const int PlanetsWidget::vertex = 0;
const int PlanetsWidget::uv     = 1;
