#include "planet.h"

QColor Planet::nextSelectionColor = QColor(1,1,1);

Planet::Planet() : position(0.0f,0.0f,0.0f) {
    mass = 1000.0f;

    selectionColor = nextSelectionColor;

    nextSelectionColor.setRed(nextSelectionColor.red()+1);
    if(nextSelectionColor.red() == 0) {
        nextSelectionColor.setRed(1);
        nextSelectionColor.setGreen(nextSelectionColor.green()+1);
        if(nextSelectionColor.green() == 0){
            nextSelectionColor.setGreen(1);
            nextSelectionColor.setBlue(nextSelectionColor.blue()+1);
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

    if(lastpathrecorddelta > 0.02f){
        path.push_back(this->position);
        lastpathrecorddelta = 0;
    }

    // doing this even if it hasn't been recorded allows it to get shorter.
    if(path.size() > pathLength){
        path.erase(path.begin());
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, &path[0]);
    glDrawArrays(GL_LINE_STRIP, 0, path.size());
}

float Planet::getRadius(){
    return pow(3.0f*mass/4.0f*M_PI, 1.0f/3.0f)/10.0f;
}

void Planet::drawBounds(GLenum drawmode, bool selectioncolor){
    bool texturing = glIsEnabled(GL_TEXTURE_2D);
    if(texturing){
        glDisable(GL_TEXTURE_2D);
    }

    if(selectioncolor){
        glColor3f(selectionColor.redF(),selectionColor.greenF(),selectionColor.blueF());
    }
    else{
        glColor3f(0.0f,1.0f,0.0f);
    }

    glPushMatrix();
    glTranslatef(position.x,position.y,position.z);

    GLUquadric *sphere = gluNewQuadric();
    gluQuadricOrientation(sphere, GLU_OUTSIDE);
    gluQuadricDrawStyle(sphere, drawmode);
    gluSphere(sphere,this->getRadius()*1.02f,32,16);

    glColor3f(1.0f,1.0f,1.0f);

    if(texturing){
        glEnable(GL_TEXTURE_2D);
    }

    glPopMatrix();
}

unsigned int Planet::pathLength = 200;
