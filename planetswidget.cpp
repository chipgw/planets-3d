#include "planetswidget.h"

PlanetsWidget::PlanetsWidget(QWidget *parent) : QGLWidget(parent) {
    setWindowTitle("Planets3D");

    this->setMouseTracking(true);

    this->doScreenshot = false;
    framerate = 60000;

    framecount = 0;
    delay=0;
    simspeed = 1.0;
    totalTime = QTime::currentTime();
    frameTime = QTime::currentTime();

    timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateGL()));

    createPlanet(glm::vec3( 3, 3,0), glm::vec3( 3,-6,-9),100);
    createPlanet(glm::vec3( 3,-3,0), glm::vec3( 3, 6, 9),100);
    createPlanet(glm::vec3(-3,-3,0), glm::vec3(-3, 6,-9),100);
    selected = createPlanet(glm::vec3(-3, 3,0), glm::vec3(-3,-6, 9),100);

    placing.position = glm::vec3(0,0,0);
    placing.velocity = glm::vec3(0,1,0);
    placing.mass = 100;
}

PlanetsWidget::~PlanetsWidget() {
    qDebug()<<"average fps: "<< framecount/totalTime.secsTo(QTime::currentTime());
}

void PlanetsWidget::initializeGL() {
    glShadeModel(GL_SMOOTH);
    qglClearColor(Qt::black);

    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);

    QFileInfo fileinfo("planet.png");
    if(!fileinfo.exists()){
        qDebug()<<"Image file "<<fileinfo.absoluteFilePath()<<" does not exist!";
    }
    else{
        QImage img(fileinfo.absoluteFilePath());
        texture = bindTexture(img);
    }
}
void PlanetsWidget::resizeGL(int width, int height) {
    if (height == 0)
        height = 1;

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(45.0f, static_cast<GLfloat>(width)/height, 0.1f, 2000.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void PlanetsWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float diffuse[] = {1.0,0.9,0.8,1.0};
    float ambient[] = {0.3,0.4,0.5,1.0};
    float position[] = {1.0,1.0,1.0,0.0};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_POSITION, position);

    glMatrixMode(GL_MODELVIEW_MATRIX);
    glLoadIdentity();
    camera.setup();

    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);

    float time = simspeed*(delay/5000.0);

    Planet *planet;
    Planet *other;
    QMutableListIterator<Planet *> i(planets);
    while (i.hasNext()) {
        planet = i.next();
        QMutableListIterator<Planet *> o(i);
        while (o.hasNext()) {
            other = o.next();

            if(other == planet)
                continue;
            else{
                glm::vec3 direction = other->position-planet->position;
                float distance = direction.x*direction.x
                        + direction.y*direction.y
                        + direction.z*direction.z;
                float frc = gravityconst*(other->mass*planet->mass/distance);

                planet->velocity += direction * frc * time / planet->mass;
                other->velocity -= direction * frc * time / other->mass;

                distance = sqrt(distance);

                if(distance < planet->getRadius()+other->getRadius()/2){
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

        planet->draw(time);
    }

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    if(selected){
        selected->drawBounds();
    }

    if(placingStep != 0){
        placing.drawBounds();
    }

    if(placingStep == 3){
        glBegin(GL_LINES);{
            glVertex3f(placing.position.x,placing.position.y,placing.position.z);
            glVertex3f(placing.position.x+placing.velocity.x,placing.position.y+placing.velocity.y,placing.position.z+placing.velocity.z);
        }glEnd();
    }

    float scale = pow(4,floor(log10(camera.distance)));

    glScalef(scale,scale,scale);

    glColor3f(0.4,0.6,0.6);
    glBegin(GL_LINES);
    for(int x = -20; x <= 20;x++){
        for(int y = -20; y <= 20;y++){
            glVertex3f(x,y,0);
            glVertex3f(x+1,y,0);
            glVertex3f(x,y,0);
            glVertex3f(x,y+1,0);

            glVertex3f(x+1,y+1,0);
            glVertex3f(x+1,y,0);
            glVertex3f(x+1,y+1,0);
            glVertex3f(x,y+1,0);
        }
    }
    glEnd();
    glColor3f(1.0,1.0,1.0);

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

    delay = frameTime.msecsTo(QTime::currentTime());
    float fps = 1000/qMax(delay+0.0,1000.0/framerate);
    framecount++;

    renderText(10,10,"simulation speed: "+QString::number(simspeed));

    if(totalTime.secsTo(QTime::currentTime()) > 0)
        renderText(10,30,"fps: "+QString::number(fps)+"\taverage fps: "+QString::number(framecount/totalTime.secsTo(QTime::currentTime())));

    if (delay == 0)
        delay = 1;
    frameTime = QTime::currentTime();

    timer->start(qMax(0, (1000/framerate) - delay));
}

void PlanetsWidget::mouseMoveEvent(QMouseEvent *e){
    if(placingStep == 1){
        // set placing XY position based on grid
        glClear(GL_DEPTH_BUFFER_BIT);
        glColorMask(0, 0, 0, 0);

        glMatrixMode(GL_MODELVIEW_MATRIX);
        glLoadIdentity();
        camera.setup();

        glBegin(GL_QUADS);{
            glVertex4f(1,1,0,0.01);
            glVertex4f(1,-1,0,0.01);
            glVertex4f(-1,-1,0,0.01);
            glVertex4f(-1,1,0,0.01);
        }glEnd();


        glm::ivec4 viewport;
        glm::mat4 modelview,projection;

        glGetIntegerv(GL_VIEWPORT, glm::value_ptr(viewport));
        glGetFloatv(GL_MODELVIEW_MATRIX, glm::value_ptr(modelview));
        glGetFloatv(GL_PROJECTION_MATRIX, glm::value_ptr(projection));

        glm::vec3 windowCoord(e->x(),viewport[3]-e->y(),0);

        glReadPixels(windowCoord.x, windowCoord.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &windowCoord.z);

        placing.position = glm::unProject(windowCoord,modelview,projection,viewport);

        glColorMask(1, 1, 1, 1);

        this->lastmousepos = e->pos();
    }
    else if(placingStep == 2){
        // set placing Z position
        placing.position.z += (lastmousepos.y() - e->y())/10.0;
        this->lastmousepos = e->pos();
    }
    else if(placingStep == 3){
        // set placing velocity
        placingXrotation += (lastmousepos.y() - e->y())/10.0f;
        placingZrotation += (lastmousepos.x() - e->x())/10.0f;
        placing.velocity = glm::rotateZ(glm::rotateX(glm::vec3(0,1,0), placingXrotation), placingZrotation) * glm::length(placing.velocity);
        QCursor::setPos(this->mapToGlobal(this->lastmousepos));
    }
    else if(e->buttons().testFlag(Qt::LeftButton)){
        this->lastmousepos = e->pos();
        this->setCursor(Qt::ArrowCursor);
    }
    else if(e->buttons().testFlag(Qt::MiddleButton)){
        float xrot = camera.xrotation + ((150.0f * (lastmousepos.y() - e->y()))/this->height());
        float zrot = camera.zrotation + ((300.0f * (lastmousepos.x() - e->x()))/this->width());
        QCursor::setPos(this->mapToGlobal(this->lastmousepos));

        xrot = qMin(qMax(xrot,-90.0f),90.0f);

        camera.xrotation = xrot;
        camera.zrotation = fmod(zrot, 360.0f);

        this->setCursor(Qt::SizeAllCursor);
    }
    else if(e->buttons().testFlag(Qt::RightButton)){

        this->setCursor(Qt::ArrowCursor);
    }
    else{
        this->setCursor(Qt::ArrowCursor);
        this->lastmousepos = e->pos();
    }
}

void PlanetsWidget::mouseDoubleClickEvent(QMouseEvent *e){
    if(e->button() == Qt::MiddleButton){
        camera.distance = 10;
        camera.xrotation = 45;
        camera.zrotation = 0;
    }
}

void PlanetsWidget::mousePressEvent(QMouseEvent *e){
    if(placingStep == 1){
        if(e->button() == Qt::LeftButton){
            placingStep = 2;
            setCursor(QCursor(Qt::BlankCursor));
        }
    }
    else if(placingStep == 2){
        if(e->button() == Qt::LeftButton){
            placingStep = 3;
        }
    }
    else if(placingStep == 3){
        if(e->button() == Qt::LeftButton){
            placingStep = 0;
            selected = createPlanet(placing.position, placing.velocity, placing.mass);
            simspeed = 1;
        }
    }
    else if(e->button() == Qt::LeftButton){
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_MODELVIEW_MATRIX);
        glLoadIdentity();
        camera.setup();

        Planet *planet;
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

void PlanetsWidget::wheelEvent(QWheelEvent *e){
    if(placingStep == 1){
        placing.mass += e->delta()*(placing.mass*1.0e-3);
        qMax(placing.mass, 0.1f);
    }
    else if(placingStep == 2){
        placing.position.z += e->delta()*camera.distance/200.0;
    }
    else if(placingStep == 3){
        placing.velocity += glm::normalize(placing.velocity) * (e->delta()*0.01f);
    }
    else {
        float dist = camera.distance - e->delta()/20.0;

        dist = qMax(dist,5.0f);
        dist = qMin(dist,300.0f);

        camera.distance = dist;
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
    QMutableListIterator<Planet *> i(planets);
    Planet *other;
    while (i.hasNext()) {
        other = i.next();
        i.remove();
        delete other;
    }
    selected = NULL;
}

void PlanetsWidget::centerAll(){
    QMutableListIterator<Planet *> i(planets);
    Planet *planet;
    glm::vec3 averagePosition(0,0,0),averageVelocity(0,0,0);
    float totalmass = 0;
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
    simspeed = 0;

    placingStep = 1;
    selected = NULL;
}
