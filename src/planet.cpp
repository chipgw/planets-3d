#include "planet.h"

QColor Planet::nextSelectionColor = QColor(1, 1, 1);

Planet::Planet() {
    mass = 1000.0f;

    selectionColor = nextSelectionColor;

    nextSelectionColor.setRed(nextSelectionColor.red() + 1);
    if(nextSelectionColor.red() == 0) {
        nextSelectionColor.setRed(1);
        nextSelectionColor.setGreen(nextSelectionColor.green() + 1);
        if(nextSelectionColor.green() == 0){
            nextSelectionColor.setGreen(1);
            nextSelectionColor.setBlue(nextSelectionColor.blue() + 1);
        }
    }
}

Planet::~Planet(){

}

void Planet::updatePath(){
    if(path.size() < 1 || (path.back() - this->position).length() > 0.05f){
        path.push_back(this->position);
    }

    // doing this even if it hasn't been recorded allows it to get shorter.
    if(path.size() > pathLength){
        path.erase(path.begin());
    }
}

float Planet::getRadius(){
    return pow(3.0f * mass / 4.0f * M_PI, 1.0f / 3.0f) * 0.1f;
}

bool Planet::operator ==(const Planet &p2){
    return this == &p2;
}

unsigned int Planet::pathLength = 200;
