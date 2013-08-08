#include "planetswidget.h"
#include <QDir>

PlanetsWidget::PlanetsWidget(QWidget* parent) : QGLWidget(QGLFormat(QGL::AccumBuffer | QGL::SampleBuffers), parent), timer(this),
    displaysettings(000), gridRange(50), following(0), doScreenshot(false), framecount(0), placingStep(None), stepsPerFrame(100),
    placing(QVector3D(), QVector3D(0.0f, velocityfac, 0.0f)), totalTime(QTime::currentTime()), frameTime(QTime::currentTime()) {

#ifndef NDEBUG
    framerate = 60000;
#else
    framerate = 60;
#endif

    this->setMouseTracking(true);

    timer.setSingleShot(true);
    connect(&timer, SIGNAL(timeout()), this, SLOT(updateGL()));

    universe.load("default.xml");
}

void PlanetsWidget::initializeGL() {
    initializeOpenGLFunctions();

    QOpenGLShader vertex(QOpenGLShader::Vertex);
    vertex.compileSourceFile(":/shaders/vertex.vsh");

    shaderTexture.addShader(&vertex);
    shaderTexture.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/texture.fsh");
    shaderTexture.link();

    shaderColor.addShader(&vertex);
    shaderColor.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/color.fsh");
    shaderColor.link();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepthf(1.0f);

#ifdef GL_ACCUM
    glClearAccum(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ACCUM_BUFFER_BIT);
#else
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#ifdef GL_LINE_SMOOTH
    glEnable(GL_LINE_SMOOTH);
#endif

    shaderColor.enableAttributeArray("vertex");

    QImage img(":/textures/planet.png");
    texture = bindTexture(img);
}

void PlanetsWidget::resizeGL(int width, int height) {
    if(height == 0){
        height = 1;
    }

    glViewport(0, 0, width, height);

    camera.projection.setToIdentity();
    camera.projection.perspective(45.0f, float(width) / float(height), 0.1f, 10000.0f);
}

void PlanetsWidget::paintGL() {
    int delay = frameTime.restart();

    if(placingStep == None){
        universe.advance(delay * 20.0f, stepsPerFrame);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // TODO - modernize motion blur...
    if(displaysettings & MotionBlur){
#ifdef GL_ACCUM
        glAccum(GL_RETURN, 1.0f);
        glClear(GL_ACCUM_BUFFER_BIT);
#endif
    }

    if(followState == Single && universe.planets.contains(following)){
        camera.position = universe.planets[following].position;
    }else if(followState == WeightedAverage){
        camera.position = QVector3D();
        float totalmass = 0.0f;

        foreach(const Planet &planet, universe.planets){
            camera.position += planet.position * planet.mass();
            totalmass += planet.mass();
        }
        camera.position /= totalmass;
    }else if(followState == PlainAverage){
        camera.position = QVector3D();

        foreach(const Planet &planet, universe.planets){
            camera.position += planet.position;
        }
        camera.position /= universe.planets.size();
    }else{
        camera.position = QVector3D();
    }

    camera.setup();

    shaderTexture.bind();
    shaderTexture.setUniformValue("cameraMatrix", camera.camera);
    shaderTexture.setUniformValue("modelMatrix", QMatrix4x4());

    shaderTexture.enableAttributeArray("uv");

    foreach(const Planet &planet, universe.planets) {
        drawPlanet(planet);
    }

    shaderTexture.disableAttributeArray("uv");

    shaderColor.bind();
    shaderColor.setUniformValue("cameraMatrix", camera.camera);
    shaderColor.setUniformValue("modelMatrix", QMatrix4x4());

    if(displaysettings & MotionBlur){
#ifdef GL_ACCUM
        glAccum(GL_ADD, -0.002f * delay);
        glAccum(GL_ACCUM, 0.999f);
#endif
    }

    if(displaysettings & PlanetColors){
        for(QMapIterator<QRgb, Planet> i(universe.planets); i.hasNext();) {
            i.next();
            drawPlanetWireframe(i.value(), i.key());
        }
    }else if(universe.planets.contains(universe.selected)){
        drawPlanetWireframe(universe.planets[universe.selected]);
    }

    if(displaysettings & PlanetTrails){
        shaderColor.setUniformValue("modelMatrix", QMatrix4x4());
        shaderColor.setUniformValue("color", QColor(Qt::white));
        foreach(const Planet &planet, universe.planets) {
            drawPlanetPath(planet);
        }
    }

    if(placingStep != None){
        drawPlanetWireframe(placing);
    }

    if(placingStep == FreeVelocity){
        float length = placing.velocity.length() / velocityfac;

        if(length > 0.0f){
            QMatrix4x4 matrix;
            matrix.translate(placing.position);
            matrix *= placingRotation;
            shaderColor.setUniformValue("modelMatrix", matrix);
            shaderColor.setUniformValue("color", QColor(Qt::white));

            float verts[] = { 0.1f, 0.1f, 0.0f,
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

                              0.0f, 0.0f, length + 0.4f};

            static const GLubyte indexes[] = {0,  1,  2,       2,  3,  0,

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
                                              8, 11, 12};

            shaderColor.setAttributeArray("vertex", GL_FLOAT, verts, 3);
            glDrawElements(GL_TRIANGLES, sizeof(indexes), GL_UNSIGNED_BYTE, indexes);
        }
    }

    QMatrix4x4 matrix;
    matrix.scale(pow(4, floor(log10(camera.distance))));
    shaderColor.setUniformValue("modelMatrix", matrix);

    if(displaysettings & SolidLineGrid){
        if(gridPoints.size() != gridRange * 4){
            gridPoints.clear();
            float bounds = (gridRange - 1) / 2.0f;
            for(float i = -bounds; i <= bounds; i++){
                gridPoints.push_back(QVector2D(i,-bounds));
                gridPoints.push_back(QVector2D(i, bounds));

                gridPoints.push_back(QVector2D(-bounds, i));
                gridPoints.push_back(QVector2D( bounds, i));
            }
        }

        shaderColor.setUniformValue("color", gridColor);

        shaderColor.setAttributeArray("vertex", GL_FLOAT, gridPoints.data(), 2);
        glDrawArrays(GL_LINES, 0, gridPoints.size());
    }
    if(displaysettings & PointGrid){
        if(gridPoints.size() != pow(gridRange, 2)){
            gridPoints.clear();
            float bounds = (gridRange - 1) / 2.0f;
            for(float x = -bounds; x <= bounds; x++){
                for(float y = -bounds; y <= bounds; y++){
                    gridPoints.push_back(QVector2D(x, y));
                }
            }
        }

        shaderColor.setUniformValue("color", gridColor);

        shaderColor.setAttributeArray("vertex", GL_FLOAT, gridPoints.data(), 2);
        glDrawArrays(GL_POINTS, 0, gridPoints.size());
    }

    if(this->doScreenshot){
        this->doScreenshot = false;

        QDir dir = QDir::homePath() + "/Pictures/Planets3D Screenshots/";
        if(!dir.exists()){
            dir.mkpath(dir.absolutePath());
        }
        QString filename = dir.path() + "/shot%1.png";
        int i = 0;
        while(QFile::exists(filename.arg(++i, 4, 10, QChar('0'))));
        filename = filename.arg(i, 4, 10, QChar('0'));

        QImage img = this->grabFrameBuffer();
        if(!img.isNull() && img.save(filename)){
            qDebug() << "Screenshot saved to:" << filename;
        }
    }

    timer.start(qMax(0, (1000 / framerate) - frameTime.elapsed()));

    framecount++;

    if(universe.planets.size() == 1){
        emit updatePlanetCountStatusMessage(tr("1 planet"));
    }else{
        emit updatePlanetCountStatusMessage(tr("%1 planets").arg(universe.planets.size()));
    }
    emit updateAverageFPSStatusMessage(tr("average fps: %1").arg(framecount / (totalTime.elapsed() * 0.001f)));
    emit updateFPSStatusMessage(tr("fps: %1").arg(1000.0f / delay));
}

void PlanetsWidget::mouseMoveEvent(QMouseEvent* e){
    if(placingStep == FreePositionXY){
        // set placing position on the XY plane
        camera.setup();

        QVector3D windowCoord((2.0f * e->x())  / width()  - 1.0f,
                              (2.0f * -e->y()) / height() + 1.0f, 0.0f);

        QMatrix4x4 inv = camera.camera.inverted();

        QVector3D origin = inv * windowCoord;

        windowCoord.setZ(1.0f);

        QVector3D ray = origin - (inv * windowCoord);

        placing.position = origin + (ray * ((-origin.z()) / ray.z()));

        this->lastmousepos = e->pos();
    }else if(placingStep == FreePositionZ){
        // set placing Z position
        placing.position.setZ(placing.position.z() + (lastmousepos.y() - e->y()) / 10.0f);
        this->lastmousepos = e->pos();
    }else if(placingStep == FreeVelocity){
        // set placing velocity
        float xdelta = (lastmousepos.x() - e->x()) / 20.0f;
        float ydelta = (lastmousepos.y() - e->y()) / 20.0f;
        placingRotation.rotate(xdelta, 1.0f, 0.0f, 0.0f);
        placingRotation.rotate(ydelta, 0.0f, 1.0f, 0.0f);
        placing.velocity = placingRotation * QVector3D(0.0f, 0.0f, placing.velocity.length());
        QCursor::setPos(this->mapToGlobal(this->lastmousepos));
    }else if(e->buttons().testFlag(Qt::MiddleButton)){
        camera.xrotation += ((150.0f * (lastmousepos.y() - e->y())) / this->height());
        camera.zrotation += ((300.0f * (lastmousepos.x() - e->x())) / this->width());
        QCursor::setPos(this->mapToGlobal(this->lastmousepos));

        camera.xrotation = qBound(-90.0f, camera.xrotation, 90.0f);
        camera.zrotation = fmod(camera.zrotation, 360.0f);

        this->setCursor(Qt::SizeAllCursor);
    }else{
        this->lastmousepos = e->pos();
    }
}

void PlanetsWidget::mouseDoubleClickEvent(QMouseEvent* e){
    if(e->button() == Qt::MiddleButton){
        camera.reset();
    }
}

void PlanetsWidget::mousePressEvent(QMouseEvent* e){
    if(e->button() == Qt::LeftButton){
        if(placingStep == FreePositionXY){
            placingStep = FreePositionZ;
            this->setCursor(Qt::BlankCursor);
        }else if(placingStep == FreePositionZ){
            placingStep = FreeVelocity;
        }else if(placingStep == FreeVelocity){
            placingStep = None;
            placing.velocity = placingRotation * QVector3D(0.0f, 0.0f, placing.velocity.length());
            universe.selected = universe.addPlanet(placing);
            this->setCursor(Qt::ArrowCursor);
        }else{
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            camera.setup();

            shaderColor.bind();
            shaderColor.setUniformValue("cameraMatrix", camera.camera);

            for(QMapIterator<QRgb, Planet> i(universe.planets); i.hasNext();) {
                i.next();
                drawPlanetColor(i.value(), i.key());
            }

            GLubyte color[4];
            glReadPixels(e->x(), height() - e->y(), 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &color);

            universe.selected = qRgba(color[0], color[1], color[2], color[3]);
        }
    }
}

void PlanetsWidget::mouseReleaseEvent(QMouseEvent *e){
    if(e->button() == Qt::MiddleButton){
        this->setCursor(Qt::ArrowCursor);
    }
}

void PlanetsWidget::wheelEvent(QWheelEvent* e){
    if(placingStep == FreePositionXY || placingStep == FreePositionZ){
        placing.setMass(qMax(placing.mass() + e->delta() * placing.mass() * 1.0e-3f, 0.1f));
    }else if(placingStep == FreeVelocity){
        placing.velocity = placingRotation * QVector3D(0.0f, 0.0f, qMax(0.0f, float(placing.velocity.length() + (e->delta() * velocityfac * 1.0e-3f))));
    }else{
        camera.distance -= e->delta() * camera.distance * 0.0005f;

        camera.distance = qBound(0.1f, camera.distance, 2000.0f);
    }
}

void PlanetsWidget::beginInteractiveCreation(){
    placingStep = FreePositionXY;
    universe.selected = 0;
}

void PlanetsWidget::drawPlanet(const Planet &planet){
    QMatrix4x4 matrix;
    matrix.translate(planet.position);
    matrix.scale(planet.radius());
    shaderTexture.setUniformValue("modelMatrix", matrix);

    shaderTexture.setAttributeArray("vertex", GL_FLOAT, highResSphere.verts.data(), 3);
    shaderTexture.setAttributeArray("uv", GL_FLOAT, highResSphere.uv.data(), 2);
    glDrawElements(GL_TRIANGLES, highResSphere.triangles.size(), GL_UNSIGNED_INT, highResSphere.triangles.data());
}

void PlanetsWidget::drawPlanetPath(const Planet &planet){
    shaderColor.setAttributeArray("vertex", GL_FLOAT, planet.path.data(), 3);
    glDrawArrays(GL_LINE_STRIP, 0, planet.path.size());
}

void PlanetsWidget::drawPlanetColor(const Planet &planet, const QRgb &color){
    shaderColor.setUniformValue("color", QColor(color));

    QMatrix4x4 matrix;
    matrix.translate(planet.position);
    matrix.scale(planet.radius() * 1.02f);
    shaderColor.setUniformValue("modelMatrix", matrix);

    shaderColor.setAttributeArray("vertex", GL_FLOAT, lowResSphere.verts.data(), 3);
    glDrawElements(GL_TRIANGLES, lowResSphere.triangles.size(), GL_UNSIGNED_INT, lowResSphere.triangles.data());
}

void PlanetsWidget::drawPlanetWireframe(const Planet &planet, const QRgb &color){
    shaderColor.setUniformValue("color", QColor(color));

    QMatrix4x4 matrix;
    matrix.translate(planet.position);
    matrix.scale(planet.radius() * 1.02f);
    shaderColor.setUniformValue("modelMatrix", matrix);

    shaderColor.setAttributeArray("vertex", GL_FLOAT, lowResSphere.verts.data(), 3);
    glDrawElements(GL_LINES, lowResSphere.lines.size(), GL_UNSIGNED_INT, lowResSphere.lines.data());
}

const QColor PlanetsWidget::gridColor = QColor(0xcc, 0xff, 0xff, 0x66);

const Sphere PlanetsWidget::highResSphere = Sphere(128, 64);
const Sphere PlanetsWidget::lowResSphere = Sphere(32, 16);
