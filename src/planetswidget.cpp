#include "planetswidget.h"
#include <QDir>
#include <qmath.h>
#include <QMouseEvent>

PlanetsWidget::PlanetsWidget(QWidget* parent) : QGLWidget(QGLFormat(QGL::SampleBuffers), parent), timer(this),
    drawGrid(false), gridRange(50), drawPlanetTrails(false), drawPlanetColors(false), following(0), doScreenshot(false),
    framecount(0), placingStep(NotPlacing), refreshRate(16), firingSpeed(PlanetsUniverse::velocityfac * 10.0f), firingMass(25.0f) {

#ifndef NDEBUG
    refreshRate = 0;
#endif

    placing.velocity.setY(PlanetsUniverse::velocityfac);

    setMouseTracking(true);

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

#ifdef PLANETS3D_USE_QOPENGLTEXTURE
    texture = new QOpenGLTexture(img.mirrored());
    texture->setMagnificationFilter(QOpenGLTexture::Linear);
    texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    texture->bind();
#else
    bindTexture(img);
#endif

    if(totalTime.isNull()){
        totalTime.start();
        frameTime.start();
    }
}

PlanetsWidget::~PlanetsWidget(){
#ifdef PLANETS3D_USE_QOPENGLTEXTURE
    delete texture;
#endif
}

void PlanetsWidget::resizeGL(int width, int height) {
    if(height == 0){
        height = 1;
    }

    glViewport(0, 0, width, height);

    camera.projection.setToIdentity();
    camera.projection.perspective(45.0f, float(width) / float(height), 0.1f, 1.0e6f);
}

void PlanetsWidget::paintGL() {
    int delay = frameTime.restart();

    if(placingStep == NotPlacing || placingStep == Firing){
        universe.advance(delay * 1000.0f);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    switch(followState){
    case WeightedAverage:{
        camera.position = QVector3D();
        float totalmass = 0.0f;

        for(PlanetsUniverse::const_iterator i = universe.begin(); i != universe.end(); ++i){
            camera.position += i.value().position * i.value().mass();
            totalmass += i.value().mass();
        }
        camera.position /= totalmass;
        break;
    }
    case PlainAverage:
        camera.position = QVector3D();

        for(PlanetsUniverse::const_iterator i = universe.begin(); i != universe.end(); ++i){
            camera.position += i.value().position;
        }
        camera.position /= universe.size();
        break;
    case Single:
        if(universe.isValid(following)){
            camera.position = universe[following].position;
            break;
        }
    default:
        camera.position = QVector3D();
    }

    shaderTexture.bind();
    shaderTexture.setUniformValue("cameraMatrix", camera.setup());
    shaderTexture.setUniformValue("modelMatrix", QMatrix4x4());

    shaderTexture.enableAttributeArray("uv");

    for(PlanetsUniverse::const_iterator i = universe.begin(); i != universe.end(); ++i){
        drawPlanet(i.value());
    }

    shaderTexture.disableAttributeArray("uv");

    shaderColor.bind();
    shaderColor.setUniformValue("cameraMatrix", camera.camera);

    if(drawPlanetColors){
        for(PlanetsUniverse::const_iterator i = universe.begin(); i != universe.end(); ++i){
            drawPlanetWireframe(i.value(), i.key());
        }
    }else if(universe.isSelectedValid()){
        drawPlanetWireframe(universe.getSelected());
    }

    if(drawPlanetTrails){
        shaderColor.setUniformValue("modelMatrix", QMatrix4x4());
        shaderColor.setUniformValue("color", trailColor);

        for(PlanetsUniverse::const_iterator i = universe.begin(); i != universe.end(); ++i){
            shaderColor.setAttributeArray("vertex", GL_FLOAT, i.value().path.data(), 3);
            glDrawArrays(GL_LINE_STRIP, 0, i.value().path.size());
        }
    }

    if(placingStep != NotPlacing && placingStep != Firing){
        drawPlanetWireframe(placing);

        if(placingStep == FreeVelocity){
            float length = placing.velocity.length() / PlanetsUniverse::velocityfac;

            if(length > 0.0f){
                QMatrix4x4 matrix;
                matrix.translate(placing.position);
                matrix.scale(placing.radius());
                matrix *= placingRotation;
                shaderColor.setUniformValue("modelMatrix", matrix);
                shaderColor.setUniformValue("color", trailColor);

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

    if((placingStep == OrbitalPlane || placingStep == OrbitalPlanet) && universe.isSelectedValid()){
        QMatrix4x4 matrix;
        matrix.translate(universe.getSelected().position);
        matrix.scale(placingOrbitalRadius);
        matrix *= placingRotation;
        shaderColor.setUniformValue("modelMatrix", matrix);
        shaderColor.setUniformValue("color", trailColor);

        shaderColor.setAttributeArray("vertex", GL_FLOAT, circle.verts, 3);
        glDrawElements(GL_LINES, circle.lineCount, GL_UNSIGNED_INT, circle.lines);
    }

    if(drawGrid){
        if(gridPoints.size() != gridRange * 4){
            updateGrid();
        }

        QMatrix4x4 matrix;
        matrix.scale(pow(4, floor(log10(camera.distance))));
        shaderColor.setUniformValue("modelMatrix", matrix);
        shaderColor.setUniformValue("color", gridColor);

        shaderColor.setAttributeArray("vertex", GL_FLOAT, gridPoints.data(), 2);
        glDrawArrays(GL_LINES, 0, gridPoints.size());
    }

    if(doScreenshot){
        doScreenshot = false;

        QDir dir = QDir::homePath() + "/Pictures/Planets3D Screenshots/";
        if(!dir.exists()){
            dir.mkpath(dir.absolutePath());
        }
        QString filename = dir.path() + "/shot%1.png";
        int i = 0;
        while(QFile::exists(filename.arg(++i, 4, 10, QChar('0'))));
        filename = filename.arg(i, 4, 10, QChar('0'));

        QImage img = grabFrameBuffer();
        if(!img.isNull() && img.save(filename)){
            qDebug("Screenshot saved to: %s", qPrintable(filename));
        }
    }

    timer.start(qMax(0, refreshRate - frameTime.elapsed()));

    framecount++;

    emit updateAverageFPSStatusMessage(tr("average fps: %1").arg(framecount * 1.0e3f / totalTime.elapsed()));
    emit updateFPSStatusMessage(tr("fps: %1").arg(1.0e3f / delay));
}

void PlanetsWidget::mouseMoveEvent(QMouseEvent* e){
    switch(placingStep){
    case FreePositionXY:{
        // set placing position on the XY plane
        QVector3D windowCoord((2.0f * e->x())  / width()  - 1.0f,
                              (2.0f * -e->y()) / height() + 1.0f, 0.0f);

        QMatrix4x4 inv = camera.setup().inverted();

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
        QCursor::setPos(mapToGlobal(lastmousepos));
        return;
    case OrbitalPlanet:
        if(universe.isSelectedValid()){
            QVector3D windowCoord((2.0f * e->x())  / width()  - 1.0f,
                                  (2.0f * -e->y()) / height() + 1.0f, 0.0f);

            QMatrix4x4 inv = camera.setup().inverted();

            QVector3D origin = inv * windowCoord;

            windowCoord.setZ(1.0f);

            QVector3D ray = origin - (inv * windowCoord);

            placing.position = origin + (ray * ((universe.getSelected().position.z() - origin.z()) / ray.z()));
            QVector3D relative = placing.position - universe.getSelected().position;
            placingOrbitalRadius = relative.length();
            placingRotation.setToIdentity();
            relative.normalize();
            placingRotation.setColumn(0, relative);
            placingRotation.setColumn(1, QVector4D(relative.y(), -relative.x(), 0.0f, 0.0f));
        }
        break;
    case OrbitalPlane:
        if(universe.isSelectedValid()){
            placingRotation.rotate((lastmousepos.x() - e->x()) * 0.05f, 1.0f, 0.0f, 0.0f);
            placingRotation.rotate((lastmousepos.y() - e->y()) * 0.05f, 0.0f, 1.0f, 0.0f);
            placing.position = universe.getSelected().position + placingRotation.column(0).toVector3D() * placingOrbitalRadius;
            QCursor::setPos(mapToGlobal(lastmousepos));
            return;
        }
        break;
    default:
        if(e->buttons().testFlag(Qt::MiddleButton) || e->buttons().testFlag(Qt::RightButton)){
            camera.xrotation += (lastmousepos.y() - e->y()) * 0.2f;
            camera.zrotation += (lastmousepos.x() - e->x()) * 0.2f;

            camera.xrotation = qBound(-90.0f, camera.xrotation, 90.0f);
            camera.zrotation = fmod(camera.zrotation, 360.0f);

            QCursor::setPos(mapToGlobal(lastmousepos));
            setCursor(Qt::SizeAllCursor);
            return;
        }
        break;
    }
    lastmousepos = e->pos();
}

void PlanetsWidget::mouseDoubleClickEvent(QMouseEvent* e){
    switch(e->button()){
    case Qt::LeftButton:
        if(universe.isSelectedValid()){
            following = universe.selected;
            followState = Single;
        }else{
            followState = FollowNone;
        }
        break;
    case Qt::MiddleButton:
        camera.reset();
        break;
    }
}

void PlanetsWidget::mousePressEvent(QMouseEvent* e){
    if(e->button() == Qt::LeftButton){
        switch(placingStep){
        case FreePositionXY:
            placingStep = FreePositionZ;
            setCursor(Qt::BlankCursor);
            break;
        case FreePositionZ:
            placingStep = FreeVelocity;
            break;
        case FreeVelocity:
            placingStep = NotPlacing;
            placing.velocity = placingRotation.column(2).toVector3D() * placing.velocity.length();
            universe.selected = universe.addPlanet(placing);
            setCursor(Qt::ArrowCursor);
            break;
        case Firing:{
            QVector3D windowCoord((2.0f * e->x())  / width()  - 1.0f,
                                  (2.0f * -e->y()) / height() + 1.0f, 0.96f);

            QMatrix4x4 inv = camera.setup().inverted();

            QVector3D origin = inv * windowCoord;

            windowCoord.setZ(0.0f);

            QVector3D velocity = origin - (inv * windowCoord);

            universe.addPlanet(Planet(origin, velocity.normalized() * firingSpeed, firingMass));

            break;
        }
        case OrbitalPlanet:
            if(universe.isSelectedValid()){
                placingStep = OrbitalPlane;
                setCursor(Qt::BlankCursor);
                break;
            }
            placingStep = NotPlacing;
            break;
        case OrbitalPlane:
            if(universe.isSelectedValid()){
                Planet &selected = universe.getSelected();
                float speed = sqrt((selected.mass() * selected.mass() * PlanetsUniverse::gravityconst) / ((selected.mass() + placing.mass()) * placingOrbitalRadius));
                QVector3D velocity = placingRotation.column(1).toVector3D() * speed;
                universe.selected = universe.addPlanet(Planet(placing.position, selected.velocity + velocity, placing.mass()));
                selected.velocity -= velocity * (placing.mass() / selected.mass());
            }
            placingStep = NotPlacing;
            setCursor(Qt::ArrowCursor);
            break;
        default:
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            shaderColor.bind();
            shaderColor.setUniformValue("cameraMatrix", camera.setup());

            for(PlanetsUniverse::const_iterator i = universe.begin(); i != universe.end(); ++i){
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
    if(e->button() == Qt::MiddleButton || e->button() == Qt::RightButton){
        setCursor(Qt::ArrowCursor);
    }
}

void PlanetsWidget::wheelEvent(QWheelEvent* e){
    switch(placingStep){
    case FreePositionXY:
    case FreePositionZ:
    case OrbitalPlanet:
    case OrbitalPlane:
        placing.setMass(qMax(placing.mass() + e->delta() * placing.mass() * 1.0e-3f, 0.01f));
        break;
    case FreeVelocity:
        placing.velocity = placingRotation.column(2).toVector3D() * qMax(0.0f, float(placing.velocity.length() + e->delta() * PlanetsUniverse::velocityfac * 1.0e-3f));
        break;
    default:
        camera.distance -= e->delta() * camera.distance * 5.0e-4f;

        camera.distance = qBound(10.0f, camera.distance, 1.0e4f);
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
    }else if(placingStep == Firing){
        placingStep = NotPlacing;
    }
}

void PlanetsWidget::beginOrbitalCreation(){
    if(universe.isSelectedValid()){
        placingStep = OrbitalPlanet;
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

void PlanetsWidget::drawPlanetColor(const Planet &planet, const QRgb &color){
    shaderColor.setUniformValue("color", QColor(color));

    QMatrix4x4 matrix;
    matrix.translate(planet.position);
    matrix.scale(planet.radius() * 1.05f);
    shaderColor.setUniformValue("modelMatrix", matrix);

    shaderColor.setAttributeArray("vertex", GL_FLOAT, lowResSphere.verts, 3);
    glDrawElements(GL_TRIANGLES, lowResSphere.triangleCount, GL_UNSIGNED_INT, lowResSphere.triangles);
}

void PlanetsWidget::drawPlanetWireframe(const Planet &planet, const QRgb &color){
    shaderColor.setUniformValue("color", QColor(color));

    QMatrix4x4 matrix;
    matrix.translate(planet.position);
    matrix.scale(planet.radius() * 1.05f);
    shaderColor.setUniformValue("modelMatrix", matrix);

    shaderColor.setAttributeArray("vertex", GL_FLOAT, lowResSphere.verts, 3);
    glDrawElements(GL_LINES, lowResSphere.lineCount, GL_UNSIGNED_INT, lowResSphere.lines);
}

void PlanetsWidget::updateGrid(){
    gridPoints.clear();

    float bounds = (gridRange - 1) / 2.0f;
    for(float i = -bounds; i <= bounds; i++){
        gridPoints.append(QVector2D(i,-bounds));
        gridPoints.append(QVector2D(i, bounds));

        gridPoints.append(QVector2D(-bounds, i));
        gridPoints.append(QVector2D( bounds, i));
    }
}

void PlanetsWidget::setGridRange(int value){
    gridRange = value;
    updateGrid();
}

const QColor PlanetsWidget::trailColor = QColor(0xcc, 0xff, 0xff, 0xff);
const QColor PlanetsWidget::gridColor = QColor(0xcc, 0xff, 0xff, 0x66);

const Sphere<64, 32> PlanetsWidget::highResSphere = Sphere<64, 32>();
const Sphere<32, 16> PlanetsWidget::lowResSphere  = Sphere<32, 16>();

const Circle<64> PlanetsWidget::circle = Circle<64>();
