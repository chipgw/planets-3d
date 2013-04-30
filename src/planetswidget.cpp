#include "planetswidget.h"
#include <QDir>

PlanetsWidget::PlanetsWidget(QWidget* parent) : QGLWidget(QGLFormat(QGL::AccumBuffer | QGL::SampleBuffers), parent), highResSphere(128, 64), lowResSphere(32, 16) {
    this->setMouseTracking(true);

    this->doScreenshot = false;

#ifndef NDEBUG
    framerate = 60000;
#else
    framerate = 60;
#endif

    framecount = 0;
    placingStep = None;
    delay = 0;
    stepsPerFrame = 100;
    totalTime = QTime::currentTime();
    frameTime = QTime::currentTime();

    timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateGL()));

    placing.position = QVector3D();
    placing.velocity = QVector3D(0.0f, velocityfac, 0.0f);
    placing.mass = 100.0f;

    displaysettings = 000;

    gridRange = 50;
    gridColor = QVector4D(0.8f, 1.0f, 1.0f, 0.4f);

    following = NULL;
    universe.load("default.xml");
}

PlanetsWidget::~PlanetsWidget() {
    qDebug()<< "average fps: " << framecount / (totalTime.msecsTo(QTime::currentTime()) * 0.001f);
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
    glClearAccum(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepthf(1.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ACCUM_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);

    glEnableVertexAttribArray(vertexAttrib);

    QImage img(":/textures/planet.png");
    texture = bindTexture(img);
}

void PlanetsWidget::resizeGL(int width, int height) {
    if (height == 0)
        height = 1;

    glViewport(0, 0, width, height);

    camera.projection = QMatrix4x4();
    camera.projection.perspective(45.0f, float(width) / float(height), 0.1f, 10000.0f);
}

void PlanetsWidget::paintGL() {
    if(placingStep == None){
        universe.advance(delay * 20.0f, stepsPerFrame);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // TODO - modernize motion blur...
    if(displaysettings & MotionBlur){
        glAccum(GL_RETURN, 1.0f);
        glClear(GL_ACCUM_BUFFER_BIT);
    }

    if(followState == Single && following != NULL){
        camera.position = following->position;
    }else if(followState == WeightedAverage){
        camera.position = QVector3D();
        float totalmass = 0.0f;

        for(QMutableListIterator<Planet> i(universe.planets); i.hasNext();) {
            Planet &planet = i.next();
            camera.position += planet.position * planet.mass;
            totalmass += planet.mass;
        }
        camera.position /= totalmass;
    }else if(followState == PlainAverage){
        camera.position = QVector3D();

        for(QMutableListIterator<Planet> i(universe.planets); i.hasNext();) {
            camera.position += i.next().position;
        }
        camera.position /= universe.planets.size();
    }
    else{
        camera.position = QVector3D();
    }

    camera.setup();

    shaderTexture.bind();
    shaderTexture.setUniformValue("cameraMatrix", camera.camera);
    shaderTexture.setUniformValue("modelMatrix", QMatrix4x4());

    glEnableVertexAttribArray(uvAttrib);

    for(QMutableListIterator<Planet> i(universe.planets); i.hasNext();) {
        drawPlanet(i.next());
    }

    glDisableVertexAttribArray(uvAttrib);

    shaderColor.bind();
    shaderColor.setUniformValue("cameraMatrix", camera.camera);
    shaderColor.setUniformValue("modelMatrix", QMatrix4x4());

    if(displaysettings & MotionBlur){
        glAccum(GL_ADD, -0.002f * delay);
        glAccum(GL_ACCUM, 0.999f);
    }

    if(universe.selected){
        drawPlanetBounds(*universe.selected);
    }

    if(displaysettings & PlanetTrails){
        shaderColor.setUniformValue("modelMatrix", QMatrix4x4());
        shaderColor.setUniformValue("color", QColor(Qt::white));
        for(QMutableListIterator<Planet> i(universe.planets); i.hasNext();) {
            drawPlanetPath(i.next());
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

            glVertexAttribPointer(vertexAttrib, 3, GL_FLOAT, GL_FALSE, 0, verts);
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

        glVertexAttribPointer(vertexAttrib, 2, GL_FLOAT, GL_FALSE, 0, &gridPoints[0]);
        glDrawArrays(GL_LINES, 0, gridPoints.size());

        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
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

        glVertexAttribPointer(vertexAttrib, 2, GL_FLOAT, GL_FALSE, 0, &gridPoints[0]);
        glDrawArrays(GL_POINTS, 0, gridPoints.size());

        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    }

    if(this->doScreenshot){
        QDir dir = QDir::homePath() + "/Pictures/Planets3D Screenshots/";
        if(!dir.exists()){
            dir.mkpath(dir.absolutePath());
        }
        QString filename = dir.path() + "/shot%1.png";
        int i = 1;
        while(QFile::exists(filename.arg(i, 4, 10, QChar('0')))){
            i++;
        }
        filename = filename.arg(i, 4, 10, QChar('0'));
        qDebug() << "Screenshot saved to: "<< filename;

        QImage img = this->grabFrameBuffer();
        img.save(filename);

        this->doScreenshot = false;
    }

    delay = qMax(frameTime.msecsTo(QTime::currentTime()), 1);
    frameTime = QTime::currentTime();

    framecount++;

    // TODO - only update the simspeed label when simspeed has changed.
    emit updateSimspeedStatusMessage(tr("simulation speed: %1").arg(universe.simspeed));
    emit updateAverageFPSStatusMessage(tr("average fps: %1").arg(framecount / (totalTime.msecsTo(QTime::currentTime()) * 0.001f)));
    emit updateFPSStatusMessage(tr("fps: %1").arg(1000.0f / delay));

    timer->start(qMax(0, (1000 / framerate) - delay));
}

void PlanetsWidget::mouseMoveEvent(QMouseEvent* e){
    if(placingStep == FreePositionXY){
        // set placing XY position based on grid
        glClear(GL_DEPTH_BUFFER_BIT);

        glColorMask(0, 0, 0, 0);
        glDisable(GL_CULL_FACE);

        camera.setup();

        shaderColor.bind();
        shaderColor.setUniformValue("cameraMatrix", camera.camera);
        shaderColor.setUniformValue("modelMatrix", QMatrix4x4());

        static const float plane[] = { 10.0f, 10.0f, 0.0f, 1.0e-5f,
                                       10.0f,-10.0f, 0.0f, 1.0e-5f,
                                      -10.0f,-10.0f, 0.0f, 1.0e-5f,
                                      -10.0f, 10.0f, 0.0f, 1.0e-5f};

        glVertexAttribPointer(vertexAttrib, 4, GL_FLOAT, GL_FALSE, 0, plane);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        GLint viewport[4];

        glGetIntegerv(GL_VIEWPORT, viewport);

        QVector3D windowCoord(e->x(), viewport[3] - e->y(), 0.0f);

        float z = 0.0f;

        glReadPixels(windowCoord.x(), windowCoord.y(), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z);

        windowCoord.setX((2 * (windowCoord.x() - viewport[0])) / viewport[2] - 1);
        windowCoord.setY((2 * (windowCoord.y() - viewport[1])) / viewport[3] - 1);
        windowCoord.setZ((2 * z) - 1);

        placing.position = camera.camera.inverted() * windowCoord;

        glEnable(GL_CULL_FACE);
        glColorMask(1, 1, 1, 1);

        this->lastmousepos = e->pos();
    }
    else if(placingStep == FreePositionZ){
        // set placing Z position
        placing.position.setZ(placing.position.z() + (lastmousepos.y() - e->y()) / 10.0f);
        this->lastmousepos = e->pos();
    }
    else if(placingStep == FreeVelocity){
        // set placing velocity
        float xdelta = (lastmousepos.x() - e->x()) / 20.0f;
        float ydelta = (lastmousepos.y() - e->y()) / 20.0f;
        placingRotation.rotate(xdelta, 1.0f, 0.0f, 0.0f);
        placingRotation.rotate(ydelta, 0.0f, 1.0f, 0.0f);
        placing.velocity = placingRotation * QVector3D(0.0f, 0.0f, placing.velocity.length());
        QCursor::setPos(this->mapToGlobal(this->lastmousepos));
    }
    else if(e->buttons().testFlag(Qt::MiddleButton)){
        camera.xrotation += ((150.0f * (lastmousepos.y() - e->y())) / this->height());
        camera.zrotation += ((300.0f * (lastmousepos.x() - e->x())) / this->width());
        QCursor::setPos(this->mapToGlobal(this->lastmousepos));

        camera.xrotation = qMin(qMax(camera.xrotation, -90.0f), 90.0f);
        camera.zrotation = fmod(camera.zrotation, 360.0f);

        this->setCursor(Qt::SizeAllCursor);
    }
    else{
        this->lastmousepos = e->pos();
    }
}

void PlanetsWidget::mouseDoubleClickEvent(QMouseEvent* e){
    if(e->button() == Qt::MiddleButton){
        camera.distance = 10.0f;
        camera.xrotation = 45.0f;
        camera.zrotation = 0.0f;
    }
}

void PlanetsWidget::mousePressEvent(QMouseEvent* e){
    if(placingStep == FreePositionXY){
        if(e->button() == Qt::LeftButton){
            placingStep = FreePositionZ;
            setCursor(QCursor(Qt::BlankCursor));
        }
    }
    else if(placingStep == FreePositionZ){
        if(e->button() == Qt::LeftButton){
            placingStep = FreeVelocity;
        }
    }
    else if(placingStep == FreeVelocity){
        if(e->button() == Qt::LeftButton){
            placingStep = None;
            universe.selected = &universe.createPlanet(placing.position, placing.velocity, placing.mass);
            this->setCursor(Qt::ArrowCursor);
        }
    }
    else if(e->button() == Qt::LeftButton){
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera.setup();

        shaderColor.bind();
        shaderColor.setUniformValue("cameraMatrix", camera.camera);

        for(QMutableListIterator<Planet> i(universe.planets); i.hasNext();) {
            drawPlanetBounds(i.next(), GL_TRIANGLES, true);
        }

        QVector4D color;
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        glReadPixels(e->x(), viewport[3] - e->y(), 1, 1, GL_RGBA, GL_FLOAT, &color);

        universe.selected = NULL;

        if(color.w() <= 0.0f){
            return;
        }
        QColor selectedcolor = QColor::fromRgbF(color.x(), color.y(), color.z());

        for(QMutableListIterator<Planet> i(universe.planets); i.hasNext();) {
            Planet *planet = &i.next();
            if(planet->selectionColor == selectedcolor){
                universe.selected = planet;
            }
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
        placing.mass += e->delta()*(placing.mass * 1.0e-3f);
        qMax(placing.mass, 0.1f);
    }
    else if(placingStep == FreeVelocity){
        placing.velocity = placingRotation * QVector3D(0.0f, 0.0f, qMax(0.0f, placing.velocity.length() + (e->delta() * velocityfac * 1.0e-3f)));
    }
    else {
        camera.distance -= e->delta() * camera.distance * 0.0005f;

        camera.distance = qMax(camera.distance, 0.1f);
        camera.distance = qMin(camera.distance, 1000.0f);
    }
}

void PlanetsWidget::beginInteractiveCreation(){
    placingStep = FreePositionXY;
    universe.selected = NULL;
}


void PlanetsWidget::drawPlanet(Planet &planet){
    QMatrix4x4 matrix;
    matrix.translate(planet.position);
    matrix.scale(planet.getRadius());
    shaderTexture.setUniformValue("modelMatrix", matrix);

    glVertexAttribPointer(vertexAttrib, 3, GL_FLOAT, GL_FALSE, 0, &highResSphere.verts[0]);
    glVertexAttribPointer(uvAttrib, 2, GL_FLOAT, GL_FALSE, 0, &highResSphere.uv[0]);
    glDrawElements(GL_TRIANGLES, highResSphere.triangles.size(), GL_UNSIGNED_INT, &highResSphere.triangles[0]);
}

void PlanetsWidget::drawPlanetPath(Planet &planet){
    glVertexAttribPointer(vertexAttrib, 3, GL_FLOAT, GL_FALSE, 0, &planet.path[0]);
    glDrawArrays(GL_LINE_STRIP, 0, planet.path.size());
}

void PlanetsWidget::drawPlanetBounds(Planet &planet, GLenum drawmode, bool selectioncolor){
    if(selectioncolor){
        shaderColor.setUniformValue("color", planet.selectionColor);
    }
    else{
        shaderColor.setUniformValue("color", QColor(Qt::green));
    }

    QMatrix4x4 matrix;
    matrix.translate(planet.position);
    matrix.scale(planet.getRadius() * 1.02f);
    shaderColor.setUniformValue("modelMatrix", matrix);

    glVertexAttribPointer(vertexAttrib, 3, GL_FLOAT, GL_FALSE, 0, &lowResSphere.verts[0]);
    if(drawmode == GL_TRIANGLES){
        glDrawElements(GL_TRIANGLES, lowResSphere.triangles.size(), GL_UNSIGNED_INT, &lowResSphere.triangles[0]);
    }else if(drawmode == GL_LINES){
        glDrawElements(GL_LINES, lowResSphere.lines.size(), GL_UNSIGNED_INT, &lowResSphere.lines[0]);
    }else{
        qDebug() << "warning, draw mode not supported!";
    }
}

const GLuint PlanetsWidget::vertexAttrib = 0;
const GLuint PlanetsWidget::uvAttrib = 1;
