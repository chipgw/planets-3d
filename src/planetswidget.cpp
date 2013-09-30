#include "planetswidget.h"
#include <QDir>
#include <qmath.h>

PlanetsWidget::PlanetsWidget(QWidget* parent) : QGLWidget(QGLFormat(QGL::SampleBuffers), parent), timer(this),
    displaysettings(0), gridRange(50), following(0), doScreenshot(false), framecount(0), placingStep(None),
    placingOrbitalRadius(1.5f), refreshRate(16), firingSpeed(velocityfac * 10.0f), firingMass(25.0f) {

#ifndef NDEBUG
    refreshRate = 0;
#endif

    placing.velocity.setY(velocityfac);

    this->setMouseTracking(true);

    timer.setSingleShot(true);
    connect(&timer, SIGNAL(timeout()), this, SLOT(updateGL()));
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

    if(totalTime.isNull()){
        totalTime.start();
        frameTime.start();
    }
}

void PlanetsWidget::resizeGL(int width, int height) {
    if(height == 0){
        height = 1;
    }

    glViewport(0, 0, width, height);

    camera.projection.setToIdentity();
    camera.projection.perspective(45.0f, float(width) / float(height), 0.1f, 1.0e4f);
}

void PlanetsWidget::paintGL() {
    int delay = frameTime.restart();

    if(placingStep == None || placingStep == Firing){
        universe.advance(delay * 1000.0f);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

    if(placingStep != None && placingStep != Firing && placingStep != OrbitalPlane){
        drawPlanetWireframe(placing);

        if(placingStep == FreeVelocity){
            float length = placing.velocity.length() / velocityfac;

            if(length > 0.0f){
                QMatrix4x4 matrix;
                matrix.translate(placing.position);
                matrix.scale(placing.radius());
                matrix *= placingRotation;
                shaderColor.setUniformValue("modelMatrix", matrix);
                shaderColor.setUniformValue("color", QColor(Qt::white));

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
    }

    if((placingStep == OrbitalPlane || placingStep == OrbitalPlanet) &&universe.planets.contains(universe.selected)){
        QMatrix4x4 matrix;
        matrix.translate(universe.planets[universe.selected].position);
        matrix.scale(universe.planets[universe.selected].radius() * placingOrbitalRadius);
        matrix *= placingRotation;
        shaderColor.setUniformValue("modelMatrix", matrix);
        shaderColor.setUniformValue("color", QColor(Qt::white));

        shaderColor.setAttributeArray("vertex", GL_FLOAT, circle.verts, 3);
        glDrawElements(GL_LINES, circle.lineCount, GL_UNSIGNED_INT, circle.lines);
    }

    QMatrix4x4 matrix;
    matrix.scale(pow(4, floor(log10(camera.distance))));
    shaderColor.setUniformValue("modelMatrix", matrix);

    if(displaysettings & SolidLineGrid){
        if(gridPoints.size() != gridRange * 4){
            updateGrid();
        }

        shaderColor.setUniformValue("color", gridColor);

        shaderColor.setAttributeArray("vertex", GL_FLOAT, gridPoints.data(), 2);
        glDrawArrays(GL_LINES, 0, gridPoints.size());
    }
    if(displaysettings & PointGrid){
        if(gridPoints.size() != gridRange * gridRange){
            updateGrid();
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

    timer.start(qMax(0, refreshRate - frameTime.elapsed()));

    framecount++;

    if(universe.planets.size() == 1){
        emit updatePlanetCountStatusMessage(tr("1 planet"));
    }else{
        emit updatePlanetCountStatusMessage(tr("%1 planets").arg(universe.planets.size()));
    }
    emit updateAverageFPSStatusMessage(tr("average fps: %1").arg(framecount / (totalTime.elapsed() * 1.0e-3f)));
    emit updateFPSStatusMessage(tr("fps: %1").arg(1.0e3f / delay));
}

void PlanetsWidget::mouseMoveEvent(QMouseEvent* e){
    switch(placingStep){
    case FreePositionXY:{
        // set placing position on the XY plane
        camera.setup();

        QVector3D windowCoord((2.0f * e->x())  / width()  - 1.0f,
                              (2.0f * -e->y()) / height() + 1.0f, 0.0f);

        QMatrix4x4 inv = camera.camera.inverted();

        QVector3D origin = inv * windowCoord;

        windowCoord.setZ(1.0f);

        QVector3D ray = origin - (inv * windowCoord);

        placing.position = origin + (ray * ((-origin.z()) / ray.z()));

        break;
    }
    case FreePositionZ:
        // set placing Z position
        placing.position.setZ(placing.position.z() + (lastmousepos.y() - e->y()) * 0.1f);
        break;
    case FreeVelocity:
        // set placing velocity
        placingRotation.rotate((lastmousepos.x() - e->x()) * 0.05f, 1.0f, 0.0f, 0.0f);
        placingRotation.rotate((lastmousepos.y() - e->y()) * 0.05f, 0.0f, 1.0f, 0.0f);
        placing.velocity = placingRotation.column(2).toVector3D() * placing.velocity.length();
        QCursor::setPos(this->mapToGlobal(this->lastmousepos));
        return;
    case OrbitalPlane:
        if(universe.planets.contains(universe.selected)){
            placingRotation.rotate((lastmousepos.x() - e->x()) * 0.05f, 1.0f, 0.0f, 0.0f);
            placingRotation.rotate((lastmousepos.y() - e->y()) * 0.05f, 0.0f, 1.0f, 0.0f);
            QCursor::setPos(this->mapToGlobal(this->lastmousepos));
            return;
        }
        break;
    case OrbitalPlanet:
        if(universe.planets.contains(universe.selected)){
            placingRotation.rotate((lastmousepos.y() - e->y()) * 0.05f, 0.0f, 0.0f, 1.0f);
            placing.position = universe.planets[universe.selected].position + placingRotation.column(0).toVector3D() * universe.planets[universe.selected].radius() * placingOrbitalRadius;
            QCursor::setPos(this->mapToGlobal(this->lastmousepos));
            return;
        }
        break;
    default:
        if(e->buttons().testFlag(Qt::MiddleButton)){
            camera.xrotation += (lastmousepos.y() - e->y()) * 0.2f;
            camera.zrotation += (lastmousepos.x() - e->x()) * 0.2f;

            camera.xrotation = qBound(-90.0f, camera.xrotation, 90.0f);
            camera.zrotation = fmod(camera.zrotation, 360.0f);

            QCursor::setPos(this->mapToGlobal(this->lastmousepos));
            this->setCursor(Qt::SizeAllCursor);
            return;
        }
    }
    this->lastmousepos = e->pos();
}

void PlanetsWidget::mouseDoubleClickEvent(QMouseEvent* e){
    if(e->button() == Qt::MiddleButton){
        camera.reset();
    }
}

void PlanetsWidget::mousePressEvent(QMouseEvent* e){
    if(e->button() == Qt::LeftButton){
        switch(placingStep){
        case FreePositionXY:
            placingStep = FreePositionZ;
            this->setCursor(Qt::BlankCursor);
            break;
        case FreePositionZ:
            placingStep = FreeVelocity;
            break;
        case FreeVelocity:
            placingStep = None;
            placing.velocity = placingRotation * QVector3D(0.0f, 0.0f, placing.velocity.length());
            universe.selected = universe.addPlanet(placing);
            this->setCursor(Qt::ArrowCursor);
            break;
        case Firing:{
            QVector3D windowCoord((2.0f * e->x())  / width()  - 1.0f,
                                  (2.0f * -e->y()) / height() + 1.0f, 0.8f);

            QMatrix4x4 inv = camera.camera.inverted();

            QVector3D origin = inv * windowCoord;

            windowCoord.setZ(0.7f);

            QVector3D velocity = origin - (inv * windowCoord);

            universe.addPlanet(Planet(origin, velocity.normalized() * firingSpeed, firingMass));

            break;
        }
        case OrbitalPlane:
            placingStep = OrbitalPlanet;
            placing.position = universe.planets[universe.selected].position + placingRotation.column(0).toVector3D() * universe.planets[universe.selected].radius() * placingOrbitalRadius;
            break;
        case OrbitalPlanet:
            if(universe.planets.contains(universe.selected)){
                const Planet &selected = universe.planets[universe.selected];
                float speed = sqrt((selected.mass() * selected.mass() * gravityconst) / ((selected.mass() + placing.mass()) * selected.radius() * placingOrbitalRadius));
                QVector3D velocity = selected.velocity + placingRotation.column(1).toVector3D() * speed;
                universe.selected = universe.addPlanet(Planet(placing.position, velocity, placing.mass()));
            }
            placingStep = None;
            break;
        default:
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
            break;
        }
    }
}

void PlanetsWidget::mouseReleaseEvent(QMouseEvent *e){
    if(e->button() == Qt::MiddleButton){
        this->setCursor(Qt::ArrowCursor);
    }
}

void PlanetsWidget::wheelEvent(QWheelEvent* e){
    switch(placingStep){
    case FreePositionXY:
    case FreePositionZ:
    case OrbitalPlanet:
        placing.setMass(qMax(placing.mass() + e->delta() * placing.mass() * 1.0e-3f, 0.01f));
        break;
    case FreeVelocity:
        placing.velocity = placingRotation * QVector3D(0.0f, 0.0f, qMax(0.0f, float(placing.velocity.length() + e->delta() * velocityfac * 1.0e-3f)));
        break;
    case OrbitalPlane:
        if(universe.planets.contains(universe.selected)){
            const Planet &selected = universe.planets[universe.selected];
            placingOrbitalRadius += e->delta() * 0.001f * placingOrbitalRadius;
            placingOrbitalRadius = qBound(1.5f, placingOrbitalRadius, 100.0f);
            placing.position = selected.position + placingRotation.column(0).toVector3D() * selected.radius() * placingOrbitalRadius;
        }
        break;
    default:
        camera.distance -= e->delta() * camera.distance * 5.0e-4f;

        camera.distance = qBound(10.0f, camera.distance, 2.0e3f);
        break;
    }
}

void PlanetsWidget::beginInteractiveCreation(){
    placingStep = FreePositionXY;
    universe.selected = 0;
}

void PlanetsWidget::enableFiringMode(bool enable){
    if(enable){
        placingStep = Firing;
        universe.selected = 0;
    }else{
        placingStep = None;
    }
}

void PlanetsWidget::beginOrbitalCreation(){
    if(universe.planets.contains(universe.selected)){
        placingStep = OrbitalPlane;
    }
}

void PlanetsWidget::takeScreenshot(){
    doScreenshot = true;
}

void PlanetsWidget::drawPlanet(const Planet &planet){
    QMatrix4x4 matrix;
    matrix.translate(planet.position);
    matrix.scale(planet.radius());
    shaderTexture.setUniformValue("modelMatrix", matrix);

    shaderTexture.setAttributeArray("vertex", GL_FLOAT, highResSphere.verts, 3);
    shaderTexture.setAttributeArray("uv", GL_FLOAT, highResSphere.uv, 2);
    glDrawElements(GL_TRIANGLES, highResSphere.triangleCount, GL_UNSIGNED_INT, highResSphere.triangles);
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

    shaderColor.setAttributeArray("vertex", GL_FLOAT, lowResSphere.verts, 3);
    glDrawElements(GL_TRIANGLES, lowResSphere.triangleCount, GL_UNSIGNED_INT, lowResSphere.triangles);
}

void PlanetsWidget::drawPlanetWireframe(const Planet &planet, const QRgb &color){
    shaderColor.setUniformValue("color", QColor(color));

    QMatrix4x4 matrix;
    matrix.translate(planet.position);
    matrix.scale(planet.radius() * 1.02f);
    shaderColor.setUniformValue("modelMatrix", matrix);

    shaderColor.setAttributeArray("vertex", GL_FLOAT, lowResSphere.verts, 3);
    glDrawElements(GL_LINES, lowResSphere.lineCount, GL_UNSIGNED_INT, lowResSphere.lines);
}

void PlanetsWidget::updateGrid(){
    gridPoints.clear();

    if(displaysettings & SolidLineGrid){
        float bounds = (gridRange - 1) / 2.0f;
        for(float i = -bounds; i <= bounds; i++){
            gridPoints.append(QVector2D(i,-bounds));
            gridPoints.append(QVector2D(i, bounds));

            gridPoints.append(QVector2D(-bounds, i));
            gridPoints.append(QVector2D( bounds, i));
        }
    }
    if(displaysettings & PointGrid){
        float bounds = (gridRange - 1) / 2.0f;
        for(float x = -bounds; x <= bounds; x++){
            for(float y = -bounds; y <= bounds; y++){
                gridPoints.append(QVector2D(x, y));
            }
        }
    }
}

void PlanetsWidget::setGridRange(int value){
    gridRange = value;
    updateGrid();
}

const QColor PlanetsWidget::gridColor = QColor(0xcc, 0xff, 0xff, 0x66);

const Sphere<128, 64> PlanetsWidget::highResSphere = Sphere<128, 64>();
const Sphere<32,  16> PlanetsWidget::lowResSphere  = Sphere<32,  16>();

const Circle<64> PlanetsWidget::circle = Circle<64>();
