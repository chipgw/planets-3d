#include "planetswidget.h"
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <glm/gtx/norm.hpp>

PlanetsWidget::PlanetsWidget(QWidget* parent) : QGLWidget(QGLFormat(QGL::AccumBuffer | QGL::SampleBuffers), parent) {
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
    simspeed = 1.0;
    stepsPerFrame = 100;
    totalTime = QTime::currentTime();
    frameTime = QTime::currentTime();

    timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateGL()));

    placing.position = glm::vec3(0.0f,0.0f,0.0f);
    placing.velocity = glm::vec3(0.0f,10.0f,0.0f);
    placing.mass = 100.0f;

    displaysettings = 000;

    gridRange = 50;
    gridColor = glm::vec4(0.8f,1.0f,1.0f,0.2f);

    selected = NULL;
    following = NULL;
    load("default.xml");
}

PlanetsWidget::~PlanetsWidget() {
    this->deleteAll();
    qDebug()<< "average fps: " << framecount/(totalTime.msecsTo(QTime::currentTime())* 0.001f);
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

    QImage img(":/textures/planet.png");
    texture = bindTexture(img);
}

void PlanetsWidget::resizeGL(int width, int height) {
    if (height == 0)
        height = 1;

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(45.0f, static_cast<GLfloat>(width)/height, 0.1f, 10000.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void PlanetsWidget::paintGL() {
    float time = 0.0f;
    if(placingStep == None){
        time = simspeed * delay * 0.001f / stepsPerFrame;

        for(int s = 0; s < stepsPerFrame; s++){
            Planet* planet;
            Planet* other;
            QMutableListIterator<Planet*> i(planets);
            while (i.hasNext()) {
                planet = i.next();
                QMutableListIterator<Planet*> o(i);
                while (o.hasNext()) {
                    other = o.next();

                    if(other == planet)
                        continue;
                    else{
                        glm::vec3 direction = other->position-planet->position;
                        float distance = glm::length2(direction);
                        float frc = gravityconst * ((other->mass * planet->mass) / distance);

                        planet->velocity += direction * frc * time / planet->mass;
                        other->velocity -= direction * frc * time / other->mass;

                        distance = sqrt(distance);

                        if(distance < planet->getRadius()+other->getRadius() / 2.0f){
                            planet->position = (other->position*other->mass + planet->position*planet->mass)/(other->mass+planet->mass);
                            planet->velocity = (other->velocity*other->mass + planet->velocity*planet->mass)/(other->mass+planet->mass);
                            planet->mass += other->mass;
                            o.remove();
                            if(other == selected){
                                selected = planet;
                            }
                            delete other;
                            planet->path.clear();
                        }
                    }
                }

                planet->position += planet->velocity * time;
            }
        }
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(displaysettings & MotionBlur){
        glAccum(GL_RETURN, 1.0f);
        glClear(GL_ACCUM_BUFFER_BIT);
    }

    glMatrixMode(GL_MODELVIEW_MATRIX);
    glLoadIdentity();

    if(followState == Single && following != NULL){
        camera.position = following->position;
    }else if(followState == WeightedAverage){
        camera.position = glm::vec3(0.0f,0.0f,0.0f);
        float totalmass = 0.0f;

        foreach(Planet *planet ,planets){
            camera.position += planet->position * planet->mass;
            totalmass += planet->mass;
        }
        camera.position /= totalmass;
    }else if(followState == PlainAverage){
        camera.position = glm::vec3(0.0f,0.0f,0.0f);

        foreach(Planet *planet ,planets){
            camera.position += planet->position;
        }
        camera.position /= planets.size();
    }
    else{
        camera.position = glm::vec3(0.0f,0.0f,0.0f);
    }

    camera.setup();

    glEnable(GL_TEXTURE_2D);

    foreach(Planet *planet, planets){
        planet->draw();
    }

    glDisable(GL_TEXTURE_2D);

    if(displaysettings & MotionBlur){
        glAccum(GL_ADD, -0.002f * delay);
        glAccum(GL_ACCUM, 0.999f);
    }

    if(selected){
        selected->drawBounds();
    }

    time *= stepsPerFrame;
    if(displaysettings & PlanetTrails){
        foreach(Planet *planet, planets){
            planet->drawPath(time);
        }
    }

    if(placingStep != None){
        placing.drawBounds();
    }

    if(placingStep == FreeVelocity){
        float length = glm::length(placing.velocity) / 20.0f;

        if(length > 0.0f){
            glPushMatrix();
            glTranslatef(placing.position.x, placing.position.y, placing.position.z);
            glMultMatrixf(glm::value_ptr(placingRotation));

            GLUquadric *cylinder = gluNewQuadric();
            gluQuadricOrientation(cylinder, GLU_OUTSIDE);
            gluCylinder(cylinder, 0.1f, 0.1f, length, 64, 1);

            GLUquadric *cap = gluNewQuadric();
            gluQuadricOrientation(cap, GLU_INSIDE);
            gluDisk(cap, 0.0f, 0.1f, 64, 1);
            glTranslatef(0.0f, 0.0f, length);
            gluCylinder(cylinder, 0.2f, 0.0f, 0.4f, 64, 1);

            gluDisk(cap, 0.1f, 0.2f, 64, 1);
            glPopMatrix();
        }

    }

    float scale = pow(4,floor(log10(camera.distance)));

    glScalef(scale,scale,scale);

    if(displaysettings & SolidLineGrid){
        glColor4fv(glm::value_ptr(gridColor));
        glBegin(GL_LINES);
        drawGrid();
        glEnd();
        glColor4f(1.0f,1.0f,1.0f,1.0f);
    }
    if(displaysettings & PointGrid){
        glColor4fv(glm::value_ptr(gridColor));
        glBegin(GL_POINTS);
        drawGrid();
        glEnd();
        glColor4f(1.0f,1.0f,1.0f,1.0f);
    }

    if(this->doScreenshot){
        QDir dir = QDir::homePath() + "/Pictures/Planets3D Screenshots/";
        if(!dir.exists()){
            dir.mkpath(dir.absolutePath());
        }
        QString filename = dir.path() + "/shot%1.png";
        int i = 1;
        while(QFile::exists(filename.arg(i,4,10,QChar('0')))){
            i++;
        }
        filename = filename.arg(i,4,10,QChar('0'));
        qDebug() << "Screenshot saved to: "<< filename;

        QImage img = this->grabFrameBuffer();
        img.save(filename);

        this->doScreenshot = false;
    }

    delay = qMax(frameTime.msecsTo(QTime::currentTime()), 1);
    frameTime = QTime::currentTime();

    float fps = 1000.0f/delay;
    framecount++;

    renderText(10,10,tr("simulation speed: %1").arg(simspeed));

    if(totalTime.msecsTo(QTime::currentTime()) > 0){
        renderText(10,30,tr("fps: %1").arg(fps));
        renderText(10,50,tr("average fps: %1").arg(framecount/(totalTime.msecsTo(QTime::currentTime()) * 0.001f)));
    }

    timer->start(qMax(0, (1000/framerate) - delay));
}

void PlanetsWidget::mouseMoveEvent(QMouseEvent* e){
    if(placingStep == FreePositionXY){
        // set placing XY position based on grid
        glClear(GL_DEPTH_BUFFER_BIT);

        glColorMask(0, 0, 0, 0);
        glDisable(GL_CULL_FACE);

        glMatrixMode(GL_MODELVIEW_MATRIX);
        glLoadIdentity();
        camera.setup();

        glBegin(GL_QUADS);{
            glVertex4f( 10.0f, 10.0f, 0.0f,1.0e-5f);
            glVertex4f( 10.0f,-10.0f, 0.0f,1.0e-5f);
            glVertex4f(-10.0f,-10.0f, 0.0f,1.0e-5f);
            glVertex4f(-10.0f, 10.0f, 0.0f,1.0e-5f);
        }glEnd();

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
        placing.velocity = glm::vec3(placingRotation * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f) * glm::length(placing.velocity));
        QCursor::setPos(this->mapToGlobal(this->lastmousepos));
    }
    else if(e->buttons().testFlag(Qt::MiddleButton)){
        float xrot = camera.xrotation + ((150.0f * (lastmousepos.y() - e->y()))/this->height());
        float zrot = camera.zrotation + ((300.0f * (lastmousepos.x() - e->x()))/this->width());
        QCursor::setPos(this->mapToGlobal(this->lastmousepos));

        xrot = glm::max(xrot,-90.0f);
        xrot = glm::min(xrot, 90.0f);

        camera.xrotation = xrot;
        camera.zrotation = fmod(zrot, 360.0f);

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
            selected = createPlanet(placing.position, placing.velocity, placing.mass);
            this->setCursor(Qt::ArrowCursor);
        }
    }
    else if(e->button() == Qt::LeftButton){
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_MODELVIEW_MATRIX);
        glLoadIdentity();
        camera.setup();

        Planet* planet;
        foreach(planet, planets){
            planet->drawBounds(GLU_FILL, true);
        }

        glm::vec4 color;
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        glReadPixels(e->x(), viewport[3] - e->y(), 1, 1, GL_RGBA, GL_FLOAT, glm::value_ptr(color));

        selected = NULL;

        if(color.a == 0){
            return;
        }
        QColor selectedcolor = QColor::fromRgbF(color.r,color.g,color.b);

        foreach(planet, planets){
            if(planet->selectionColor == selectedcolor){
                this->selected = planet;
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
        placing.mass += e->delta()*(placing.mass*1.0e-3f);
        glm::max(placing.mass, 0.1f);
    }
    else if(placingStep == FreeVelocity){
        placing.velocity = glm::vec3(placingRotation * glm::vec4(0.0f,0.0f,1.0f, 1.0f) * glm::max(0.0f, glm::length(placing.velocity) + (e->delta()*0.01f)));
    }
    else {
        camera.distance -= e->delta() * camera.distance * 0.0005f;

        camera.distance = glm::max(camera.distance, 5.0f);
        camera.distance = glm::min(camera.distance, 1000.0f);
    }
}

Planet* PlanetsWidget::createPlanet(glm::vec3 position, glm::vec3 velocity, float mass){
    Planet* planet = new Planet();
    planet->position = position;
    planet->velocity = velocity;
    planet->mass = mass;
    planets.append(planet);
    return planet;
}

void PlanetsWidget::deleteAll(){
    QMutableListIterator<Planet*> i(planets);
    Planet* planet;
    while (i.hasNext()) {
        planet = i.next();
        i.remove();
        delete planet;
    }
    selected = NULL;
}

void PlanetsWidget::centerAll(){
    QMutableListIterator<Planet* > i(planets);
    Planet* planet;
    glm::vec3 averagePosition(0.0f,0.0f,0.0f),averageVelocity(0.0f,0.0f,0.0f);
    float totalmass = 0.0f;
    while (i.hasNext()) {
        planet = i.next();

        averagePosition += planet->position * planet->mass;
        averageVelocity += planet->velocity * planet->mass;
        totalmass += planet->mass;
    }
    averagePosition /= totalmass;
    averageVelocity /= totalmass;

    i.toFront();
    while (i.hasNext()) {
        planet = i.next();

        planet->position -= averagePosition;
        planet->velocity -= averageVelocity;
        planet->path.clear();
    }
}

void PlanetsWidget::beginInteractiveCreation(){
    placingStep = FreePositionXY;
    selected = NULL;
}

void PlanetsWidget::drawGrid(){
    for(int x = -gridRange; x <= gridRange;x++){
        for(int y = -gridRange; y <= gridRange;y++){
            glVertex3f(x,     y,     0.0f);
            glVertex3f(x+1.0f,y,     0.0f);
            glVertex3f(x,     y,     0.0f);
            glVertex3f(x,     y+1.0f,0.0f);

            glVertex3f(x+1.0f,y+1.0f,0.0f);
            glVertex3f(x+1.0f,y,     0.0f);
            glVertex3f(x+1.0f,y+1.0f,0.0f);
            glVertex3f(x,     y+1.0f,0.0f);
        }
    }
}

bool PlanetsWidget::load(const QString &filename){
    if(!QFile::exists(filename)){
        qDebug(qPrintable(tr("\"%1\" does not exist!").arg(filename)));
        return false;
    }
    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        return false;
    }

    QXmlStreamReader xml(&file);

    if(xml.readNextStartElement()) {
        if (xml.name() == "planets-3d-universe"){
            deleteAll();
            while(xml.readNextStartElement()) {
                if(xml.name() == "planet"){
                    Planet* planet = new Planet();

                    planet->mass = xml.attributes().value("mass").toString().toFloat();

                    while(xml.readNextStartElement()){
                        if(xml.name() == "position"){
                            planet->position.x = xml.attributes().value("x").toString().toFloat();
                            planet->position.y = xml.attributes().value("y").toString().toFloat();
                            planet->position.z = xml.attributes().value("z").toString().toFloat();
                            xml.readNext();
                        }
                        if(xml.name() == "velocity"){
                            planet->velocity.x = xml.attributes().value("x").toString().toFloat();
                            planet->velocity.y = xml.attributes().value("y").toString().toFloat();
                            planet->velocity.z = xml.attributes().value("z").toString().toFloat();
                            xml.readNext();
                        }
                    }
                    planets.append(planet);
                    xml.readNext();
                }
            }
            if(xml.hasError()){
                qDebug(qPrintable(tr("\"%1\" had error: %2").arg(filename).arg(xml.errorString())));
                return false;
            }
        }

        else{
            qDebug(qPrintable(tr("\"%1\" is not a valid universe file!").arg(filename)));
            return false;
        }
    }


    return true;
}

bool PlanetsWidget::save(const QString &filename){
    QFile file(filename);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        return false;
    }

    QXmlStreamWriter xml(&file);

    xml.writeStartDocument();
    xml.writeStartElement("planets-3d-universe");

    QMutableListIterator<Planet* > i(planets);
    Planet* planet;
    while (i.hasNext()) {
        planet = i.next();

        xml.writeStartElement("planet");

        xml.writeAttribute("mass", QString::number(planet->mass));

        xml.writeStartElement("position"); {
            xml.writeAttribute("x", QString::number(planet->position.x));
            xml.writeAttribute("y", QString::number(planet->position.y));
            xml.writeAttribute("z", QString::number(planet->position.z));
        } xml.writeEndElement();

        xml.writeStartElement("velocity"); {
            xml.writeAttribute("x", QString::number(planet->velocity.x));
            xml.writeAttribute("y", QString::number(planet->velocity.y));
            xml.writeAttribute("z", QString::number(planet->velocity.z));
        } xml.writeEndElement();

        xml.writeEndElement();
    }
    xml.writeEndElement();

    xml.writeEndDocument();

    return true;
}
