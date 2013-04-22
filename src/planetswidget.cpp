#include "planetswidget.h"
#include <QDir>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>

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

    placing.position = glm::vec3(0.0f);
    placing.velocity = glm::vec3(0.0f, velocityfac, 0.0f);
    placing.mass = 100.0f;

    displaysettings = 000;

    gridRange = 50;
    gridColor = glm::vec4(0.8f, 1.0f, 1.0f, 0.4f);

    following = NULL;
    universe.load("default.xml");
}

PlanetsWidget::~PlanetsWidget() {
    qDebug()<< "average fps: " << framecount / (totalTime.msecsTo(QTime::currentTime()) * 0.001f);
}

void PlanetsWidget::initializeGL() {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearAccum(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ACCUM_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);

    glEnableClientState(GL_VERTEX_ARRAY);

    QImage img(":/textures/planet.png");
    texture = bindTexture(img);
}

void PlanetsWidget::resizeGL(int width, int height) {
    if (height == 0)
        height = 1;

    glViewport(0, 0, width, height);

    camera.projection = glm::perspective(45.0f, float(width) / float(height), 0.1f, 10000.0f);
}

void PlanetsWidget::paintGL() {
    if(placingStep == None){
        universe.advance(delay * 20.0f, stepsPerFrame);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(displaysettings & MotionBlur){
        glAccum(GL_RETURN, 1.0f);
        glClear(GL_ACCUM_BUFFER_BIT);
    }

    if(followState == Single && following != NULL){
        camera.position = following->position;
    }else if(followState == WeightedAverage){
        camera.position = glm::vec3(0.0f);
        float totalmass = 0.0f;

        for(QMutableListIterator<Planet> i(universe.planets); i.hasNext();) {
            Planet &planet = i.next();
            camera.position += planet.position * planet.mass;
            totalmass += planet.mass;
        }
        camera.position /= totalmass;
    }else if(followState == PlainAverage){
        camera.position = glm::vec3(0.0f);

        for(QMutableListIterator<Planet> i(universe.planets); i.hasNext();) {
            camera.position += i.next().position;
        }
        camera.position /= universe.planets.size();
    }
    else{
        camera.position = glm::vec3(0.0f);
    }

    glMatrixMode(GL_PROJECTION);

    camera.setup();
    glLoadMatrixf(glm::value_ptr(camera.camera));

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_TEXTURE_2D);

    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    for(QMutableListIterator<Planet> i(universe.planets); i.hasNext();) {
        drawPlanet(i.next());
    }

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glDisable(GL_TEXTURE_2D);

    if(displaysettings & MotionBlur){
        glAccum(GL_ADD, -0.002f * delay);
        glAccum(GL_ACCUM, 0.999f);
    }

    if(universe.selected){
        drawPlanetBounds(*universe.selected);
    }

    if(displaysettings & PlanetTrails){
        for(QMutableListIterator<Planet> i(universe.planets); i.hasNext();) {
            drawPlanetPath(i.next());
        }
    }

    if(placingStep != None){
        drawPlanetBounds(placing);
    }

    if(placingStep == FreeVelocity){
        float length = glm::length(placing.velocity) / velocityfac;

        if(length > 0.0f){
            glLoadIdentity();
            glTranslatef(placing.position.x, placing.position.y, placing.position.z);
            glMultMatrixf(glm::value_ptr(placingRotation));

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

            glVertexPointer(3, GL_FLOAT, 0, verts);
            glDrawElements(GL_TRIANGLES, sizeof(indexes), GL_UNSIGNED_BYTE, indexes);
        }

    }

    float scale = pow(4, floor(log10(camera.distance)));

    glScalef(scale, scale, scale);

    if(displaysettings & SolidLineGrid){
        if(gridPoints.size() != (gridRange * 2 + 1) * 4){
            gridPoints.clear();
            for(int i = -gridRange; i <= gridRange; i++){
                gridPoints.push_back(glm::vec2(i,-gridRange));
                gridPoints.push_back(glm::vec2(i, gridRange));

                gridPoints.push_back(glm::vec2(-gridRange, i));
                gridPoints.push_back(glm::vec2( gridRange, i));
            }
        }

        glColor4fv(glm::value_ptr(gridColor));

        glVertexPointer(2, GL_FLOAT, 0, &gridPoints[0]);
        glDrawArrays(GL_LINES, 0, gridPoints.size());

        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    }
    if(displaysettings & PointGrid){
        if(gridPoints.size() != pow(gridRange * 2 + 1, 2)){
            gridPoints.clear();
            for(int x = -gridRange; x <= gridRange; x++){
                for(int y = -gridRange; y <= gridRange; y++){
                    gridPoints.push_back(glm::vec2(x, y));
                }
            }
        }

        glColor4fv(glm::value_ptr(gridColor));

        glVertexPointer(2, GL_FLOAT, 0, &gridPoints[0]);
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

        glMatrixMode(GL_PROJECTION);

        camera.setup();
        glLoadMatrixf(glm::value_ptr(camera.camera));

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        static const float plane[] = { 10.0f, 10.0f, 0.0f, 1.0e-5f,
                                       10.0f,-10.0f, 0.0f, 1.0e-5f,
                                      -10.0f,-10.0f, 0.0f, 1.0e-5f,
                                      -10.0f, 10.0f, 0.0f, 1.0e-5f};

        glVertexPointer(4, GL_FLOAT, 0, plane);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glm::ivec4 viewport;
        glm::mat4 modelview,projection;

        glGetIntegerv(GL_VIEWPORT, glm::value_ptr(viewport));
        glGetFloatv(GL_MODELVIEW_MATRIX, glm::value_ptr(modelview));
        glGetFloatv(GL_PROJECTION_MATRIX, glm::value_ptr(projection));

        glm::vec3 windowCoord(e->x(),viewport[3]-e->y(),0);

        glReadPixels(windowCoord.x, windowCoord.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &windowCoord.z);

        placing.position = glm::unProject(windowCoord,modelview,projection,viewport);

        glEnable(GL_CULL_FACE);
        glColorMask(1, 1, 1, 1);

        this->lastmousepos = e->pos();
    }
    else if(placingStep == FreePositionZ){
        // set placing Z position
        placing.position.z += (lastmousepos.y() - e->y()) / 10.0f;
        this->lastmousepos = e->pos();
    }
    else if(placingStep == FreeVelocity){
        // set placing velocity
        float xdelta = (lastmousepos.x() - e->x()) / 20.0f;
        float ydelta = (lastmousepos.y() - e->y()) / 20.0f;
        placingRotation *= glm::rotate(xdelta, 1.0f, 0.0f, 0.0f);
        placingRotation *= glm::rotate(ydelta, 0.0f, 1.0f, 0.0f);
        placing.velocity = glm::vec3(placingRotation * glm::vec4(0.0f, 0.0f, glm::length(placing.velocity), 0.0f));
        QCursor::setPos(this->mapToGlobal(this->lastmousepos));
    }
    else if(e->buttons().testFlag(Qt::MiddleButton)){
        camera.xrotation += ((150.0f * (lastmousepos.y() - e->y())) / this->height());
        camera.zrotation += ((300.0f * (lastmousepos.x() - e->x())) / this->width());
        QCursor::setPos(this->mapToGlobal(this->lastmousepos));

        camera.xrotation = glm::min(glm::max(camera.xrotation, -90.0f), 90.0f);
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

        glMatrixMode(GL_PROJECTION);

        camera.setup();
        glLoadMatrixf(glm::value_ptr(camera.camera));

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        for(QMutableListIterator<Planet> i(universe.planets); i.hasNext();) {
            drawPlanetBounds(i.next(), GL_TRIANGLES, true);
        }

        glm::vec4 color;
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        glReadPixels(e->x(), viewport[3] - e->y(), 1, 1, GL_RGBA, GL_FLOAT, glm::value_ptr(color));

        universe.selected = NULL;

        if(color.a == 0){
            return;
        }
        QColor selectedcolor = QColor::fromRgbF(color.r,color.g,color.b);

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
        glm::max(placing.mass, 0.1f);
    }
    else if(placingStep == FreeVelocity){
        placing.velocity = glm::vec3(placingRotation * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f) * glm::max(0.0f, glm::length(placing.velocity) + (e->delta() * velocityfac * 1.0e-3f)));
    }
    else {
        camera.distance -= e->delta() * camera.distance * 0.0005f;

        camera.distance = glm::max(camera.distance, 0.1f);
        camera.distance = glm::min(camera.distance, 1000.0f);
    }
}

void PlanetsWidget::beginInteractiveCreation(){
    placingStep = FreePositionXY;
    universe.selected = NULL;
}


void PlanetsWidget::drawPlanet(Planet &planet){
    glLoadIdentity();
    glTranslatef(planet.position.x, planet.position.y, planet.position.z);
    float r = planet.getRadius();
    glScalef(r, r, r);

    glVertexPointer(3, GL_FLOAT, 0, &highResSphere.verts[0]);
    glTexCoordPointer(2, GL_FLOAT, 0, &highResSphere.uv[0]);
    glDrawElements(GL_TRIANGLES, highResSphere.triangles.size(), GL_UNSIGNED_INT, &highResSphere.triangles[0]);
}

void PlanetsWidget::drawPlanetPath(Planet &planet){
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, &planet.path[0]);
    glDrawArrays(GL_LINE_STRIP, 0, planet.path.size());
}

void PlanetsWidget::drawPlanetBounds(Planet &planet, GLenum drawmode, bool selectioncolor){
    if(selectioncolor){
        glColor3f(planet.selectionColor.redF(), planet.selectionColor.greenF(), planet.selectionColor.blueF());
    }
    else{
        glColor3f(0.0f, 1.0f, 0.0f);
    }

    glLoadIdentity();
    glTranslatef(planet.position.x, planet.position.y, planet.position.z);
    float r = planet.getRadius() * 1.02f;
    glScalef(r, r, r);

    glVertexPointer(3, GL_FLOAT, 0, &lowResSphere.verts[0]);
    if(drawmode == GL_TRIANGLES){
        glTexCoordPointer(2, GL_FLOAT, 0, &lowResSphere.uv[0]);
        glDrawElements(GL_TRIANGLES, lowResSphere.triangles.size(), GL_UNSIGNED_INT, &lowResSphere.triangles[0]);
    }else if(drawmode == GL_LINES){
        glDrawElements(GL_LINES, lowResSphere.lines.size(), GL_UNSIGNED_INT, &lowResSphere.lines[0]);
    }else{
        qDebug() << "warning, draw mode not supported!";
    }

    glColor3f(1.0f, 1.0f, 1.0f);
}
