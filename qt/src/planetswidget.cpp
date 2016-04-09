#include "planetswidget.h"
#include <QDir>
#include <QMouseEvent>
#include <QOpenGLFramebufferObject>
#include <QApplication>
#include <limits>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>

PlanetsWidget::PlanetsWidget(QWidget* parent) : QOpenGLWidget(parent), placing(universe), camera(universe),
    screenshotDir(QDir::homePath() + "/Pictures/Planets3D-Screenshots/"), highResSphereTris(QOpenGLBuffer::IndexBuffer),
#ifdef PLANETS3D_QT_USE_SDL_GAMEPAD
    gamepad(universe, camera, placing),
#endif
    lowResSphereLines(QOpenGLBuffer::IndexBuffer), circleLines(QOpenGLBuffer::IndexBuffer) {
    /* We want mouse movement events. */
    setMouseTracking(true);

    /* Don't let people make the widget really small. */
    setMinimumSize(QSize(100, 100));

#ifdef PLANETS3D_QT_USE_SDL_GAMEPAD
    gamepad.initSDL();

    gamepad.closeFunction = &QApplication::closeAllWindows;
#endif
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

    texture = new QOpenGLTexture(img);

    /* Begin vertex/index buffer allocation. */

    const static Sphere<64, 32> highResSphere;
    const static Sphere<32, 16> lowResSphere;
    const static Circle<64> circle;

    highResSphereVerts.create();
    highResSphereVerts.bind();
    highResSphereVerts.allocate(highResSphere.verts, highResSphere.vertexCount * sizeof(Vertex));

    highResSphereTris.create();
    highResSphereTris.bind();
    highResSphereTris.allocate(highResSphere.triangles, highResSphere.triangleCount * sizeof(unsigned int));
    highResSphereTriCount = highResSphere.triangleCount;

    lowResSphereVerts.create();
    lowResSphereVerts.bind();
    lowResSphereVerts.allocate(lowResSphere.verts, lowResSphere.vertexCount * sizeof(Vertex));

    lowResSphereLines.create();
    lowResSphereLines.bind();
    lowResSphereLines.allocate(lowResSphere.lines, lowResSphere.lineCount * sizeof(unsigned int));
    lowResSphereLineCount = lowResSphere.lineCount;

    circleVerts.create();
    circleVerts.bind();
    circleVerts.allocate(circle.verts, circle.vertexCount * sizeof(Vertex));

    circleLines.create();
    circleLines.bind();
    circleLines.allocate(circle.lines, circle.lineCount * sizeof(unsigned int));
    circleLineCount = circle.lineCount;

    QOpenGLBuffer::release(QOpenGLBuffer::VertexBuffer);
    QOpenGLBuffer::release(QOpenGLBuffer::IndexBuffer);

    /* End vertex/index buffer allocation. */

    /* If we haven't rendered any frames yet, start the timer. */
    if (frameCount == 0) {
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

#ifdef PLANETS3D_QT_USE_SDL_GAMEPAD
    gamepad.pollGamepad();
    gamepad.doControllerAxisInput(delay);
#endif

    /* Don't advance if placing. */
    if (placing.step == PlacingInterface::NotPlacing || placing.step == PlacingInterface::Firing)
        universe.advance(delay);

    render();

    update();

    emit updateAverageFPSStatusMessage(tr("average fps: %1").arg(++frameCount * 1.0e3f / totalTime.elapsed()));
    emit updateFPSStatusMessage(tr("fps: %1").arg(1.0e6f / delay));
}

void PlanetsWidget::render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    camera.setup();

    if (!hidePlanets) {
        /* Only used for drawing the planets. */
        shaderTexture.bind();

        /* Upload the updated camera matrix. */
        glUniformMatrix4fv(shaderTexture_cameraMatrix, 1, GL_FALSE, glm::value_ptr(camera.camera));

        shaderTexture.enableAttributeArray(uv);

        /* Our one and only texture, which apparently doesn't stay bound between frames... */
        texture->bind();

        highResSphereVerts.bind();
        highResSphereTris.bind();

        /* Set up the attribute buffers once for all planets. */
        shaderTexture.setAttributeBuffer(vertex, GL_FLOAT, 0,                       3, sizeof(Vertex));
        shaderTexture.setAttributeBuffer(uv,     GL_FLOAT, offsetof(Vertex, uv),    2, sizeof(Vertex));

        for (const auto& i : universe) {
            /* Set up a matrix for the planet's position and size. */
            glm::mat4 matrix = glm::translate(i.second.position);
            matrix = glm::scale(matrix, glm::vec3(i.second.radius() * drawScale));
            glUniformMatrix4fv(shaderTexture_modelMatrix, 1, GL_FALSE, glm::value_ptr(matrix));

            /* Draw the high resolution sphere. */
            glDrawElements(GL_TRIANGLES, highResSphereTriCount, GL_UNSIGNED_INT, nullptr);
        }

        highResSphereVerts.release();
        highResSphereTris.release();

        /* That's the only thing that uses the uv coords. */
        shaderTexture.disableAttributeArray(uv);
    }

    /* Everything else uses the color shader. */
    shaderColor.bind();

    /* Upload the updated camera matrix. */
    glUniformMatrix4fv(shaderColor_cameraMatrix, 1, GL_FALSE, glm::value_ptr(camera.camera));

    lowResSphereVerts.bind();
    lowResSphereLines.bind();

    if (drawPlanetColors)
        for (const auto& i : universe)
            drawPlanetWireframe(i.second, i.first);
    else if (!hidePlanets && universe.isSelectedValid())
        drawPlanetWireframe(universe.getSelected());

    lowResSphereVerts.release();
    lowResSphereLines.release();

    if (drawPlanetTrails) {
        shaderColor.setUniformValue(shaderColor_modelMatrix, QMatrix4x4());
        shaderColor.setUniformValue(shaderColor_color, trailColor);

        for (const auto& i : universe) {
            shaderColor.setAttributeArray(vertex, GL_FLOAT, i.second.path.data(), 3);
            glDrawArrays(GL_LINE_STRIP, 0, GLsizei(i.second.path.size()));
        }
    }

    switch (placing.step) {
    case PlacingInterface::FreeVelocity: {
        float length = glm::length(placing.planet.velocity) / universe.velocityfac;

        if (length > 0.0f) {
            glm::mat4 matrix = glm::translate(placing.planet.position);
            matrix = glm::scale(matrix, glm::vec3(placing.planet.radius()));
            matrix *= placing.rotation;
            glUniformMatrix4fv(shaderColor_modelMatrix, 1, GL_FALSE, glm::value_ptr(matrix));
            shaderColor.setUniformValue(shaderColor_color, trailColor);

            /* A simple arrow. */
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
        lowResSphereVerts.bind();
        lowResSphereLines.bind();

        drawPlanetWireframe(placing.planet);

        lowResSphereVerts.release();
        lowResSphereLines.release();
        break;
    case PlacingInterface::OrbitalPlane:
    case PlacingInterface::OrbitalPlanet:
        if (universe.isSelectedValid() && placing.orbitalRadius > 0.0f) {
            /* This is how large the planet being placed's orbit will be. */
            float newPlanetRadius = placing.orbitalRadius / (1 + (placing.planet.mass() / universe.getSelected().mass()));
            /* This is how large the planet being orbited around will be displaced by the new planet. */
            float oldPlanetRadius = placing.orbitalRadius / (1 + (universe.getSelected().mass() / placing.planet.mass()));

            /* Start at the placing planet location and move -1 (* the radius value) on x to locate the center. */
            glm::mat4 newRadiusMatrix = glm::translate(placing.planet.position);
            newRadiusMatrix = glm::scale(newRadiusMatrix, glm::vec3(newPlanetRadius));
            newRadiusMatrix *= placing.rotation;
            newRadiusMatrix = glm::translate(newRadiusMatrix, glm::vec3(-1.0f, 0.0f, 0.0f));

            /* Start at the selected planet location and move 1 (* the radius value) on x to locate the center. */
            glm::mat4 oldRadiusMatrix = glm::translate(universe.getSelected().position);
            oldRadiusMatrix = glm::scale(oldRadiusMatrix, glm::vec3(oldPlanetRadius));
            oldRadiusMatrix *= placing.rotation;
            oldRadiusMatrix = glm::translate(oldRadiusMatrix, glm::vec3( 1.0f, 0.0f, 0.0f));

            circleVerts.bind();
            circleLines.bind();

            shaderColor.setUniformValue(shaderColor_color, trailColor);
            shaderColor.setAttributeBuffer(vertex, GL_FLOAT, 0, 3, sizeof(Vertex));

            /* Draw both circles. */
            glUniformMatrix4fv(shaderColor_modelMatrix, 1, GL_FALSE, glm::value_ptr(newRadiusMatrix));
            glDrawElements(GL_LINES, circleLineCount, GL_UNSIGNED_INT, nullptr);

            glUniformMatrix4fv(shaderColor_modelMatrix, 1, GL_FALSE, glm::value_ptr(oldRadiusMatrix));
            glDrawElements(GL_LINES, circleLineCount, GL_UNSIGNED_INT, nullptr);

            circleVerts.release();
            circleLines.release();

            lowResSphereVerts.bind();
            lowResSphereLines.bind();

            drawPlanetWireframe(placing.planet);

            lowResSphereVerts.release();
            lowResSphereLines.release();
        }
        break;
    default: break;
    }

    if (grid.draw) {
        grid.update(camera);

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

#ifdef PLANETS3D_QT_USE_SDL_GAMEPAD
    /* If there is a controller attached, we aren't placing, and we aren't following anything, draw a little circle in the center of the screen. */
    if (gamepad.isAttached() && placing.step == PlacingInterface::NotPlacing && camera.followingState == Camera::FollowNone) {
        glDisable(GL_DEPTH_TEST);

        glUniform4fv(shaderColor_color, 1, glm::value_ptr(glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)));

        circleVerts.bind();
        circleLines.bind();

        glm::mat4 matrix = glm::translate(camera.position);
        matrix = glm::scale(matrix, glm::vec3(camera.distance * 4.0e-3f));
        glUniformMatrix4fv(shaderColor_modelMatrix, 1, GL_FALSE, glm::value_ptr(matrix));

        shaderColor.setAttributeBuffer(vertex, GL_FLOAT, 0, 3, sizeof(Vertex));
        glDrawElements(GL_LINES, circleLineCount, GL_UNSIGNED_INT, nullptr);

        circleVerts.release();
        circleLines.release();

        glEnable(GL_DEPTH_TEST);
    }
#endif
}

void PlanetsWidget::takeScreenshot() {
    QElapsedTimer t;
    t.start();

    makeCurrent();

    QString filename = screenshotDir.absoluteFilePath("shot%1.png");
    int i = 0;
    while (QFile::exists(filename.arg(++i, 4, 10, QChar('0'))));
    filename = filename.arg(i, 4, 10, QChar('0'));

    QOpenGLFramebufferObjectFormat fmt;
    /* Skip the Alpha component. */
    fmt.setInternalTextureFormat(GL_RGB);
    /* We need a depth buffer. */
    fmt.setAttachment(QOpenGLFramebufferObject::Depth);

    /* Basically we just want as many as possible if they're supported. */
    if (context()->hasExtension("GL_EXT_framebuffer_multisample") && context()->hasExtension("GL_EXT_framebuffer_blit"))
        fmt.setSamples(64);

    /* TODO - It would be easy enough to make this support arbitrarily sized screenshots.
     * The camera and viewport would have to be resized though, and there would need to be UI for it... */
    QOpenGLFramebufferObject sample(this->size(), fmt);
    sample.bind();

    render();

    /* Get the image to save. */
    QImage img = sample.toImage();

    if (!img.isNull() && img.save(filename))
        statusBarMessage(("Screenshot saved to: \"" + filename + "\", operation took %1ms").arg(t.elapsed()) , 10000);

    doneCurrent();
}

void PlanetsWidget::mouseMoveEvent(QMouseEvent* e) {
    glm::ivec2 delta(lastMousePos.x() - e->x(), lastMousePos.y() - e->y());

    bool holdCursor = false;

    if (!placing.handleMouseMove(glm::ivec2(e->x(), e->y()), delta, camera, holdCursor)) {
        if (e->buttons().testFlag(Qt::MiddleButton)) {
            camera.distance -= delta.y * camera.distance * 1.0e-2f;
            setCursor(Qt::SizeVerCursor);
            camera.bound();
        } else if (e->buttons().testFlag(Qt::RightButton)) {
            camera.xrotation += delta.y * 0.01f;
            camera.zrotation += delta.x * 0.01f;

            camera.bound();

            holdCursor = true;
        }
    }
    if (holdCursor) {
        QCursor::setPos(mapToGlobal(lastMousePos));
        setCursor(Qt::BlankCursor);
    } else {
        lastMousePos = e->pos();
    }
}

void PlanetsWidget::mouseDoubleClickEvent(QMouseEvent* e) {
    switch(e->button()) {
    case Qt::LeftButton:
        /* Double clicking the left button while not placing sets or clears the planet currently being followed. */
        if (placing.step == PlacingInterface::NotPlacing) {
            if (universe.isSelectedValid()) {
                camera.followSelection();
            } else {
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

void PlanetsWidget::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton) {
        glm::ivec2 pos(e->x(), e->y());

        /* Send click to placement system. If it doesn't use it and planets aren't hidden, select under the cursor. */
        if (!placing.handleMouseClick(pos, camera) && !hidePlanets)
            camera.selectUnder(pos, drawScale);
    }
}

void PlanetsWidget::mouseReleaseEvent(QMouseEvent* e) {
    setCursor(Qt::ArrowCursor);
}

void PlanetsWidget::wheelEvent(QWheelEvent* e) {
    if (!placing.handleMouseWheel(e->delta() * 1.0e-3f)) {
        camera.distance -= e->delta() * camera.distance * 5.0e-4f;

        camera.bound();
    }
}

void PlanetsWidget::drawPlanetWireframe(const Planet& planet, const QColor& color) {
    shaderColor.setUniformValue(shaderColor_color, color);

    glm::mat4 matrix = glm::translate(planet.position);
    /* Wireframe scales to 1.05x the normal scale, so that it will be above the surface of a planet. */
    matrix = glm::scale(matrix, glm::vec3(planet.radius() * drawScale * 1.05f));
    glUniformMatrix4fv(shaderColor_modelMatrix, 1, GL_FALSE, glm::value_ptr(matrix));

    /* No need for UV... */
    shaderColor.setAttributeBuffer(vertex, GL_FLOAT, 0, 3, sizeof(Vertex));
    glDrawElements(GL_LINES, lowResSphereLineCount, GL_UNSIGNED_INT, nullptr);
}

const QColor PlanetsWidget::trailColor = QColor(0xcc, 0xff, 0xff, 0xff);

const int PlanetsWidget::vertex = 0;
const int PlanetsWidget::uv     = 1;
