#include "planetswidget.h"
#include <QDir>
#include <QMouseEvent>
#include <limits>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>

#ifdef PLANETS3D_USE_QOPENGLTEXTURE
#include <QOpenGLTexture>
#endif

int highBit(unsigned int n) {
    n |= (n >>  1);
    n |= (n >>  2);
    n |= (n >>  4);
    n |= (n >>  8);
    n |= (n >> 16);
    return n - (n >> 1);
}

PlanetsWidget::PlanetsWidget(QWidget* parent) : QGLWidget(QGLFormat(QGL::SampleBuffers), parent),
    doScreenshot(false), frameCount(0), refreshRate(16), timer(this), placingStep(NotPlacing), gridRange(32),
    following(0), firingSpeed(PlanetsUniverse::velocityfac * 10.0f), firingMass(25.0f), drawScale(1.0f),
    drawGrid(false), drawPlanetTrails(false), drawPlanetColors(false), hidePlanets(false),
    screenshotDir(QDir::homePath() + "/Pictures/Planets3D-Screenshots/") {

    if(!screenshotDir.exists()){
        QDir oldDir = QDir::homePath() + "/Pictures/Planets3D Screenshots/";
        if(oldDir.exists()){
            oldDir.rename(oldDir.absolutePath(), screenshotDir.absolutePath());
        }else{
            screenshotDir.mkpath(screenshotDir.absolutePath());
        }
    }

#ifndef NDEBUG
    refreshRate = 0;
#endif

    placing.velocity.y = PlanetsUniverse::velocityfac;

    setMouseTracking(true);

    timer.setSingleShot(true);
    connect(&timer, SIGNAL(timeout()), this, SLOT(updateGL()));

    setMinimumSize(QSize(100, 100));
}

void PlanetsWidget::initializeGL() {
    initializeOpenGLFunctions();

    shaderTexture.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/texture.vsh");
    shaderTexture.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/texture.fsh");

    shaderTexture.bindAttributeLocation("vertex", vertex);
    shaderTexture.bindAttributeLocation("uv", uv);

    shaderTexture.link();

    shaderTexture_cameraMatrix = shaderTexture.uniformLocation("cameraMatrix");
    shaderTexture_modelMatrix = shaderTexture.uniformLocation("modelMatrix");

    shaderColor.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/color.vsh");
    shaderColor.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/color.fsh");

    shaderColor.bindAttributeLocation("vertex", vertex);

    shaderColor.link();

    shaderColor_cameraMatrix = shaderColor.uniformLocation("cameraMatrix");
    shaderColor_modelMatrix = shaderColor.uniformLocation("modelMatrix");
    shaderColor_color = shaderColor.uniformLocation("color");

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

    shaderColor.enableAttributeArray(vertex);

    QImage img(":/textures/planet.png");

#ifdef PLANETS3D_USE_QOPENGLTEXTURE
    texture = new QOpenGLTexture(img.mirrored());
    texture->setMagnificationFilter(QOpenGLTexture::Linear);
    texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    texture->bind();
#else
    bindTexture(img);
#endif

    if(frameCount == 0){
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
    glViewport(0, 0, width, height);

    camera.projection = glm::perspective(45.0f, float(width) / float(height), 0.1f, 1.0e6f);
}

void PlanetsWidget::paintGL() {
    int delay = frameTime.nsecsElapsed() / 1000;
    frameTime.start();

    if(placingStep == NotPlacing || placingStep == Firing){
        universe.advance(delay);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    switch(followState){
    case WeightedAverage:
        camera.position = glm::vec3();

        if(universe.size() != 0){
            float totalmass = 0.0f;

            for(const auto& i : universe){
                camera.position += i.second.position * i.second.mass();
                totalmass += i.second.mass();
            }
            camera.position /= totalmass;
        }
        break;
    case PlainAverage:
        camera.position = glm::vec3();

        if(universe.size() != 0){
            for(const auto& i : universe){
                camera.position += i.second.position;
            }
            camera.position /= universe.size();
        }
        break;
    case Single:
        if(universe.isValid(following)){
            camera.position = universe[following].position;
            break;
        }
    default:
        camera.position = glm::vec3();
    }

    camera.setup();

    if(!hidePlanets){
        shaderTexture.bind();
        glUniformMatrix4fv(shaderTexture_cameraMatrix, 1, GL_FALSE, glm::value_ptr(camera.camera));
        shaderTexture.setUniformValue(shaderTexture_modelMatrix, QMatrix4x4());

        shaderTexture.enableAttributeArray(uv);

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
            glDrawArrays(GL_LINE_STRIP, 0, i.second.path.size());
        }
    }

    switch(placingStep){
    case FreeVelocity:{
        float length = glm::length(placing.velocity) / PlanetsUniverse::velocityfac;

        if(length > 0.0f){
            glm::mat4 matrix = glm::translate(placing.position);
            matrix = glm::scale(matrix, glm::vec3(placing.radius()));
            matrix *= placingRotation;
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
    case FreePositionXY:
    case FreePositionZ:
        drawPlanetWireframe(placing);
        break;
    case OrbitalPlane:
    case OrbitalPlanet:
        if(universe.isSelectedValid() && placingOrbitalRadius > 0.0f){
            glm::mat4 matrix = glm::translate(universe.getSelected().position);
            matrix = glm::scale(matrix, glm::vec3(placingOrbitalRadius));
            matrix *= placingRotation;
            glUniformMatrix4fv(shaderColor_modelMatrix, 1, GL_FALSE, glm::value_ptr(matrix));
            shaderColor.setUniformValue(shaderColor_color, trailColor);

            shaderColor.setAttributeArray(vertex, GL_FLOAT, circle.verts, 3, sizeof(Vertex));
            glDrawElements(GL_LINES, circle.lineCount, GL_UNSIGNED_INT, circle.lines);

            drawPlanetWireframe(placing);
        }
        break;
    default: break;
    }

    if(drawGrid){
        if(gridPoints.size() != (gridRange + 1) * 8){
            updateGrid();
        }

        glDepthMask(GL_FALSE);

        shaderColor.setAttributeArray(vertex, GL_FLOAT, gridPoints.data(), 2);

        float distance = pow(glm::length2(camera.camera[3]), 1.0f / 3.0f);
        int nearestPowerOfTwo = highBit(distance);
        float alphafac = distance / nearestPowerOfTwo - 1.0f;

        QColor color = gridColor;
        color.setAlphaF(gridColor.alphaF() * alphafac);

        QMatrix4x4 matrix;
        matrix.scale(nearestPowerOfTwo);
        shaderColor.setUniformValue(shaderColor_modelMatrix, matrix);
        shaderColor.setUniformValue(shaderColor_color, color);

        glDrawArrays(GL_LINES, 0, gridPoints.size() / 2);

        matrix.scale(0.5f);
        shaderColor.setUniformValue(shaderColor_modelMatrix, matrix);
        color.setAlphaF(gridColor.alphaF() * (1.0f - alphafac));
        shaderColor.setUniformValue(shaderColor_color, color);

        glDrawArrays(GL_LINES, 0, gridPoints.size() / 2);

        glDepthMask(GL_TRUE);
    }

    if(doScreenshot){
        doScreenshot = false;

        QString filename = screenshotDir.absoluteFilePath("shot%1.png");
        int i = 0;
        while(QFile::exists(filename.arg(++i, 4, 10, QChar('0'))));
        filename = filename.arg(i, 4, 10, QChar('0'));

        QImage img = grabFrameBuffer();
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
    switch(placingStep){
    case FreePositionXY:{
        // set placing position on the XY plane
        Ray ray = camera.getRay(e->pos().x(), e->pos().y(), width(), height(), false);

        placing.position = ray.origin + (ray.direction * ((-ray.origin.z) / ray.direction.z));
        break;
    }
    case FreePositionZ:
        // set placing Z position
        placing.position.z += (lastMousePos.y() - e->y()) * 0.1f;
        QCursor::setPos(mapToGlobal(lastMousePos));
        return;
    case FreeVelocity:
        // set placing velocity
        placingRotation *= glm::rotate((lastMousePos.x() - e->x()) * 0.05f, glm::vec3(1.0f, 0.0f, 0.0f));
        placingRotation *= glm::rotate((lastMousePos.y() - e->y()) * 0.05f, glm::vec3(0.0f, 1.0f, 0.0f));
        placing.velocity = glm::vec3(placingRotation[2]) * glm::length(placing.velocity);
        QCursor::setPos(mapToGlobal(lastMousePos));
        return;
    case OrbitalPlanet:
        if(universe.isSelectedValid()){
            Ray ray = camera.getRay(e->pos().x(), e->pos().y(), width(), height(), false);

            placing.position = ray.origin + (ray.direction * ((universe.getSelected().position.z - ray.origin.z) / ray.direction.z));
            glm::vec3 relative = placing.position - universe.getSelected().position;
            placingOrbitalRadius = glm::length(relative);
            relative /= placingOrbitalRadius;
            placingRotation = glm::mat4(glm::vec4(relative, 0.0f),
                                        glm::vec4(relative.y, -relative.x, 0.0f, 0.0f),
                                        glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
                                        glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        }
        break;
    case OrbitalPlane:
        if(universe.isSelectedValid()){
            placingRotation *= glm::rotate((lastMousePos.x() - e->x()) * 0.05f, glm::vec3(1.0f, 0.0f, 0.0f));
            placingRotation *= glm::rotate((lastMousePos.y() - e->y()) * 0.05f, glm::vec3(0.0f, 1.0f, 0.0f));
            placing.position = universe.getSelected().position + glm::vec3(placingRotation[0] * placingOrbitalRadius);
            QCursor::setPos(mapToGlobal(lastMousePos));
            return;
        }
        break;
    default:
        if(e->buttons().testFlag(Qt::MiddleButton) || e->buttons().testFlag(Qt::RightButton)){
            if(e->modifiers().testFlag(Qt::ControlModifier)){
                camera.distance -= (lastMousePos.y() - e->y()) * camera.distance * 1.0e-2f;
                setCursor(Qt::SizeVerCursor);
                camera.bound();
            }else{
                camera.xrotation += (lastMousePos.y() - e->y()) * 0.2f;
                camera.zrotation += (lastMousePos.x() - e->x()) * 0.2f;

                camera.bound();

                QCursor::setPos(mapToGlobal(lastMousePos));
                setCursor(Qt::SizeAllCursor);
                return;
            }
        }
        break;
    }
    lastMousePos = e->pos();
}

void PlanetsWidget::mouseDoubleClickEvent(QMouseEvent* e){
    switch(e->button()){
    case Qt::LeftButton:
        if(placingStep == NotPlacing){
            if(universe.isSelectedValid()){
                following = universe.selected;
                followState = Single;
            }else{
                followState = FollowNone;
            }
        }
        break;
    case Qt::MiddleButton:
    case Qt::RightButton:
        camera.reset();
        break;
    default: break;
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
            placing.velocity = glm::vec3(placingRotation[2]) * glm::length(placing.velocity);
            universe.selected = universe.addPlanet(placing);
            setCursor(Qt::ArrowCursor);
            break;
        case Firing:{
            Ray ray = camera.getRay(e->pos().x(), e->pos().y(), width(), height(), true, 0.96f, 0.0f);

            universe.addPlanet(Planet(ray.origin, ray.direction * firingSpeed, firingMass));
            break;
        }
        case OrbitalPlanet:
            if(universe.isSelectedValid()){
                placingStep = OrbitalPlane;
                setCursor(Qt::BlankCursor);
                break;
            }
            placingOrbitalRadius = 0.0f;
            placingStep = NotPlacing;
            break;
        case OrbitalPlane:
            if(universe.isSelectedValid()){
                Planet &selected = universe.getSelected();
                float speed = sqrt((selected.mass() * selected.mass() * PlanetsUniverse::gravityconst) / ((selected.mass() + placing.mass()) * placingOrbitalRadius));
                glm::vec3 velocity = glm::vec3(placingRotation[1]) * speed;
                universe.selected = universe.addPlanet(Planet(placing.position, selected.velocity + velocity, placing.mass()));
                selected.velocity -= velocity * (placing.mass() / selected.mass());
            }
            placingOrbitalRadius = 0.0f;
            placingStep = NotPlacing;
            setCursor(Qt::ArrowCursor);
            break;
        default:
            universe.resetSelected();
            float nearest = -std::numeric_limits<float>::max();

            Ray ray = camera.getRay(e->pos().x(), e->pos().y(), width(), height(), true);

            for(const auto& i : universe){
                glm::vec3 difference = i.second.position - ray.origin;
                float dot = glm::dot(difference, ray.direction);
                if(dot > nearest && (glm::length2(difference) - dot * dot) <= (i.second.radius() * i.second.radius())) {
                    universe.selected = i.first;
                    nearest = dot;
                }
            }
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
        placing.setMass(glm::clamp(placing.mass() + e->delta() * placing.mass() * 1.0e-3f, PlanetsUniverse::min_mass, PlanetsUniverse::max_mass));
        break;
    case FreeVelocity:
        placing.velocity = glm::vec3(placingRotation[2]) * qMax(0.0f, glm::length(placing.velocity) + e->delta() * PlanetsUniverse::velocityfac * 1.0e-3f);
        break;
    default:
        camera.distance -= e->delta() * camera.distance * 5.0e-4f;

        camera.bound();
        break;
    }
}

void PlanetsWidget::beginInteractiveCreation(){
    placingStep = FreePositionXY;
    universe.resetSelected();
}

void PlanetsWidget::enableFiringMode(bool enable){
    if(enable){
        placingStep = Firing;
        universe.resetSelected();
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

void PlanetsWidget::updateGrid(){
    gridPoints.clear();

    float bounds = gridRange / 2.0f;
    for(float i = -bounds; i <= bounds; ++i){
        gridPoints.push_back(i); gridPoints.push_back(-bounds);
        gridPoints.push_back(i); gridPoints.push_back( bounds);

        gridPoints.push_back(-bounds); gridPoints.push_back(i);
        gridPoints.push_back( bounds); gridPoints.push_back(i);
    }
}

void PlanetsWidget::setGridRange(int value){
    gridRange = value;
    updateGrid();
}

void PlanetsWidget::followNext(){
    if(!universe.isEmpty()){
        followState = Single;
        PlanetsUniverse::const_iterator current = universe.find(following);

        if(current == universe.cend()){
            current = universe.cbegin();
        }else if(++current == universe.cend()){
            current = universe.cbegin();
        }

        following = current->first;
    }
}
void PlanetsWidget::followPrevious(){
    if(!universe.isEmpty()){
        followState = Single;
        PlanetsUniverse::const_iterator current = universe.find(following);

        if(current == universe.cend()){
            current = universe.cbegin();
        }else{
            if(current == universe.cbegin()){
                current = universe.cend();
            }
            --current;
        }

        following = current->first;
    }
}

const QColor PlanetsWidget::trailColor = QColor(0xcc, 0xff, 0xff, 0xff);
const QColor PlanetsWidget::gridColor = QColor(0xcc, 0xff, 0xff, 0x66);

const Sphere<64, 32> PlanetsWidget::highResSphere = Sphere<64, 32>();
const Sphere<32, 16> PlanetsWidget::lowResSphere  = Sphere<32, 16>();

const Circle<64> PlanetsWidget::circle = Circle<64>();

const int PlanetsWidget::vertex = 0;
const int PlanetsWidget::uv     = 1;
