#include "planet.h"

QColor Planet::nextSelectionColor = QColor(1,1,1);

Planet::Planet() : position(0,0,0) {
    mass = 1000;

    selectionColor = nextSelectionColor;

    nextSelectionColor.setRed(nextSelectionColor.red()+10);
    if(nextSelectionColor.red() == 0) {
        nextSelectionColor.setRed(1);
        nextSelectionColor.setGreen(nextSelectionColor.green()+10);
        if(nextSelectionColor.green() == 0){
            nextSelectionColor.setGreen(1);
            nextSelectionColor.setBlue(nextSelectionColor.blue()+10);
        }
    }
    lastpathrecorddelta = 0;
}

Planet::~Planet(){

}

void Planet::draw(){
    glPushMatrix();
    glTranslatef(position.x,position.y,position.z);

    GLUquadric *sphere = gluNewQuadric();
    gluQuadricTexture(sphere, GL_TRUE);
    gluQuadricOrientation(sphere, GLU_OUTSIDE);
    gluSphere(sphere,this->getRadius(),64,32);

    glPopMatrix();
}

void Planet::drawPath(float time){
    lastpathrecorddelta += time;

    glDisable(GL_TEXTURE_2D);

    if(lastpathrecorddelta > 0.02){
        path.push_back(this->position);

        if(path.size() > 100){
            path.erase(path.begin());
        }
        lastpathrecorddelta = 0;
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, &path[0]);
    glDrawArrays(GL_LINE_STRIP, 0, path.size());

    glEnable(GL_TEXTURE_2D);
}

float Planet::getRadius(){
    return pow(3*mass/4*M_PI, 1.0/3.0)/10;
}

void Planet::drawBounds(GLenum drawmode, bool selectioncolor){
    bool lighting = glIsEnabled(GL_LIGHTING);
    bool texturing = glIsEnabled(GL_TEXTURE_2D);
    if(lighting)
        glDisable(GL_LIGHTING);
    if(texturing)
        glDisable(GL_TEXTURE_2D);

    if(selectioncolor){
        glColor3f(selectionColor.redF(),selectionColor.greenF(),selectionColor.blueF());
    }
    else{
        glColor3f(0.0,1.0,0.0);
    }

    glPushMatrix();
    glTranslatef(position.x,position.y,position.z);

    GLUquadric *sphere = gluNewQuadric();
    gluQuadricOrientation(sphere, GLU_OUTSIDE);
    gluQuadricDrawStyle(sphere, drawmode);
    gluSphere(sphere,this->getRadius()*1.02,32,16);

    glColor3f(1.0,1.0,1.0);

    if(lighting)
        glEnable(GL_LIGHTING);
    if(texturing)
        glEnable(GL_TEXTURE_2D);

    glPopMatrix();
}
