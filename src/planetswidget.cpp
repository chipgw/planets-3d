#include "planetswidget.h"
#include <QDir>

PlanetsWidget::PlanetsWidget(QWidget* parent) : QGLWidget(QGLFormat(QGL::AccumBuffer | QGL::SampleBuffers), parent), highResSphere(128, 64), lowResSphere(32, 16), timer(this),
    displaysettings(000), gridRange(50), gridColor(0.8f, 1.0f, 1.0f, 0.4f), following(0), doScreenshot(false), framecount(0), placingStep(None), delay(0), stepsPerFrame(100),
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

#ifdef GL_ACCUM_BUFFER_BIT
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
    if(placingStep == None){
        universe.advance(delay * 20.0f, stepsPerFrame);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // TODO - modernize motion blur...
    if(displaysettings & MotionBlur){
#ifdef GL_ACCUM_BUFFER_BIT
        glAccum(GL_RETURN, 1.0f);
        glClear(GL_ACCUM_BUFFER_BIT);
#endif
    }

    if(followState == Single && universe.planets.contains(following)){
        camera.position = universe.planets[following].position;
    }else if(followState == WeightedAverage){
        camera.position = QVector3D();
        float totalmass = 0.0f;

        for(QMutableMapIterator<QRgb, Planet> i(universe.planets); i.hasNext();) {
            Planet &planet = i.next().value();
            camera.position += planet.position * planet.mass;
            totalmass += planet.mass;
        }
        camera.position /= totalmass;
    }else if(followState == PlainAverage){
        camera.position = QVector3D();

        for(QMutableMapIterator<QRgb, Planet> i(universe.planets); i.hasNext();) {
            camera.position += i.next().value().position;
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

    for(QMutableMapIterator<QRgb, Planet> i(universe.planets); i.hasNext();) {
        drawPlanet(i.next().value());
    }

    shaderTexture.disableAttributeArray("uv");

    shaderColor.bind();
    shaderColor.setUniformValue("cameraMatrix", camera.camera);
    shaderColor.setUniformValue("modelMatrix", QMatrix4x4());

    if(displaysettings & MotionBlur){
#ifdef GL_ACCUM_BUFFER_BIT
        glAccum(GL_ADD, -0.002f * delay);
        glAccum(GL_ACCUM, 0.999f);
#endif
    }

    if(displaysettings & PlanetColors){
        for(QMutableMapIterator<QRgb, Planet> i(universe.planets); i.hasNext();) {
            i.next();
            drawPlanetBounds(i.value(), false, i.key());
        }
    }else if(universe.planets.contains(universe.selected)){
        drawPlanetBounds(universe.planets[universe.selected]);
    }

    if(displaysettings & PlanetTrails){
        shaderColor.setUniformValue("modelMatrix", QMatrix4x4());
        shaderColor.setUniformValue("color", QColor(Qt::white));
        for(QMutableMapIterator<QRgb, Planet> i(universe.planets); i.hasNext();) {
            drawPlanetPath(i.next().value());
        }
    }

    if(placingStep != None){
        drawPlanetBounds(placing);
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
        if(gridPoints.size() != (gridRange * 2 + 1) * 4){
            gridPoints.clear();
            for(int i = -gridRange; i <= gridRange; i++){
                gridPoints.push_back(QVector2D(i,-gridRange));
                gridPoints.push_back(QVector2D(i, gridRange));

                gridPoints.push_back(QVector2D(-gridRange, i));
                gridPoints.push_back(QVector2D( gridRange, i));
            }
        }

        shaderColor.setUniformValue("color", gridColor);

        shaderColor.setAttributeArray("vertex", GL_FLOAT, gridPoints.data(), 2);
        glDrawArrays(GL_LINES, 0, gridPoints.size());
    }
    if(displaysettings & PointGrid){
        if(gridPoints.size() != pow(gridRange * 2 + 1, 2)){
            gridPoints.clear();
            for(int x = -gridRange; x <= gridRange; x++){
                for(int y = -gridRange; y <= gridRange; y++){
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

    delay = qMax(frameTime.msecsTo(QTime::currentTime()), 1);
    frameTime = QTime::currentTime();

    framecount++;

    if(universe.planets.size() == 1){
        emit updatePlanetCountStatusMessage(tr("1 planet"));
    }else{
        emit updatePlanetCountStatusMessage(tr("%1 planets").arg(universe.planets.size()));
    }
    emit updateAverageFPSStatusMessage(tr("average fps: %1").arg(framecount / (totalTime.msecsTo(QTime::currentTime()) * 0.001f)));
    emit updateFPSStatusMessage(tr("fps: %1").arg(1000.0f / delay));

    timer.start(qMax(0, (1000 / framerate) - delay));
}

void PlanetsWidget::mouseMoveEvent(QMouseEvent* e){
    if(placingStep == FreePositionXY){
        // set placing XY position based on grid
        camera.setup();

        GLint viewport[4];

        glGetIntegerv(GL_VIEWPORT, viewport);

        QVector3D windowCoord((2.0f * (e->x() - viewport[0])) / viewport[2] - 1.0f,
                (2.0f * ((viewport[3] - e->y()) - viewport[1])) / viewport[3] - 1.0f, 0.0f);

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

        camera.xrotation = qMin(qMax(camera.xrotation, -90.0f), 90.0f);
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
    if(placingStep == FreePositionXY){
        if(e->button() == Qt::LeftButton){
            placingStep = FreePositionZ;
            setCursor(QCursor(Qt::BlankCursor));
        }
    }else if(placingStep == FreePositionZ){
        if(e->button() == Qt::LeftButton){
            placingStep = FreeVelocity;
        }
    }else if(placingStep == FreeVelocity){
        if(e->button() == Qt::LeftButton){
            placingStep = None;
            placing.velocity = placingRotation * QVector3D(0.0f, 0.0f, placing.velocity.length());
            universe.selected = universe.addPlanet(placing);
            this->setCursor(Qt::ArrowCursor);
        }
    }else if(e->button() == Qt::LeftButton){
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera.setup();

        shaderColor.bind();
        shaderColor.setUniformValue("cameraMatrix", camera.camera);

        for(QMutableMapIterator<QRgb, Planet> i(universe.planets); i.hasNext();) {
            i.next();
            drawPlanetBounds(i.value(), true, i.key());
        }

        QVector4D color;
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        glReadPixels(e->x(), viewport[3] - e->y(), 1, 1, GL_RGBA, GL_FLOAT, &color);

        universe.selected = qRgba(color.x() * 0xff, color.y() * 0xff, color.z() * 0xff, color.w() * 0xff);
    }
}

void PlanetsWidget::mouseReleaseEvent(QMouseEvent *e){
    if(e->button() == Qt::MiddleButton){
        this->setCursor(Qt::ArrowCursor);
    }
}

void PlanetsWidget::wheelEvent(QWheelEvent* e){
    if(placingStep == FreePositionXY || placingStep == FreePositionZ){
        placing.mass += e->delta() * placing.mass * 1.0e-3f;
        qMax(placing.mass, 0.1f);
    }else if(placingStep == FreeVelocity){
        placing.velocity = placingRotation * QVector3D(0.0f, 0.0f, qMax(0.0f, float(placing.velocity.length() + (e->delta() * velocityfac * 1.0e-3f))));
    }else{
        camera.distance -= e->delta() * camera.distance * 0.0005f;

        camera.distance = qMax(camera.distance, 0.1f);
        camera.distance = qMin(camera.distance, 2000.0f);
    }
}

void PlanetsWidget::beginInteractiveCreation(){
    placingStep = FreePositionXY;
    universe.selected = 0;
}

void PlanetsWidget::drawPlanet(Planet &planet){
    QMatrix4x4 matrix;
    matrix.translate(planet.position);
    matrix.scale(planet.getRadius());
    shaderTexture.setUniformValue("modelMatrix", matrix);

    shaderTexture.setAttributeArray("vertex", GL_FLOAT, highResSphere.verts.data(), 3);
    shaderTexture.setAttributeArray("uv", GL_FLOAT, highResSphere.uv.data(), 2);
    glDrawElements(GL_TRIANGLES, highResSphere.triangles.size(), GL_UNSIGNED_INT, highResSphere.triangles.data());
}

void PlanetsWidget::drawPlanetPath(Planet &planet){
    shaderColor.setAttributeArray("vertex", GL_FLOAT, planet.path.data(), 3);
    glDrawArrays(GL_LINE_STRIP, 0, planet.path.size());
}

void PlanetsWidget::drawPlanetBounds(Planet &planet, bool triangles, QRgb color){
    shaderColor.setUniformValue("color", QColor(color));

    QMatrix4x4 matrix;
    matrix.translate(planet.position);
    matrix.scale(planet.getRadius() * 1.02f);
    shaderColor.setUniformValue("modelMatrix", matrix);

    shaderColor.setAttributeArray("vertex", GL_FLOAT, lowResSphere.verts.data(), 3);
    if(triangles){
        glDrawElements(GL_TRIANGLES, lowResSphere.triangles.size(), GL_UNSIGNED_INT, lowResSphere.triangles.data());
    }else{
        glDrawElements(GL_LINES, lowResSphere.lines.size(), GL_UNSIGNED_INT, lowResSphere.lines.data());
    }
}
