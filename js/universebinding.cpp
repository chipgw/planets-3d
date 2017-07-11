#include <planetsuniverse.h>
#include <planet.h>
#include "glbindings.h"

/* Wrap addPlanet to avoid having to create an instance of Planet in JS,
 * because then memory managment becomes an issue. */
key_type createPlanet(PlanetsUniverse& universe) {
    return universe.addPlanet(Planet());
}

key_type createPlanetPos(PlanetsUniverse& universe, glm::vec3 position) {
    return universe.addPlanet(Planet(position));
}

key_type createPlanetPosVel(PlanetsUniverse& universe, glm::vec3 position, glm::vec3 velocity) {
    return universe.addPlanet(Planet(position, velocity));
}

key_type createPlanetAll(PlanetsUniverse& universe, glm::vec3 position, glm::vec3 velocity, float mass) {
    return universe.addPlanet(Planet(position, velocity, mass));
}

/* Need to wrap these functions because I can't pass Planets without getting them copied for some reason... */
glm::vec3 getPlanetPosition(PlanetsUniverse& universe, key_type key) {
    return universe[key].position;
}
glm::vec3 getPlanetVelocity(PlanetsUniverse& universe, key_type key) {
    return universe[key].velocity;
}
float getPlanetMass(PlanetsUniverse& universe, key_type key) {
    return universe[key].mass();
}
float getPlanetRadius(PlanetsUniverse& universe, key_type key) {
    return universe[key].radius();
}
void setPlanetPosition(PlanetsUniverse& universe, key_type key, glm::vec3 pos) {
    universe[key].position = pos;
}
void setPlanetVelocity(PlanetsUniverse& universe, key_type key, glm::vec3 vel) {
    universe[key].velocity = vel;
}
void setPlanetMass(PlanetsUniverse& universe, key_type key, float mass) {
    universe[key].setMass(mass);
}

void drawTrails(PlanetsUniverse& universe) {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glEnableVertexAttribArray(vertex);

    for (const auto& i : universe) {
        glVertexAttribPointer(vertex, 3, GL_FLOAT, GL_FALSE, 0, i.path.data());
        glDrawArrays(GL_LINE_STRIP, 0, GLsizei(i.path.size()));
    }
}

float getVelocityFac(PlanetsUniverse&) {
    return PlanetsUniverse::velocityFactor;
}

/* Wrap to avoid problems with the optional replacement parameter.
 * (Emscripten requires it to be passed, and we can't pass -1 due to type differences between JS and C++.) */
void removePlanet(PlanetsUniverse& universe, key_type planet) {
    universe.remove(planet);
}

EMSCRIPTEN_BINDINGS(planets_universe) {
    emscripten::class_<PlanetsUniverse>("PlanetsUniverse")
            .constructor()
            .function("addPlanet",              &createPlanet)
            .function("addPlanet",              &createPlanetPos)
            .function("addPlanet",              &createPlanetPosVel)
            .function("addPlanet",              &createPlanetAll)
            .function("getPlanetPosition",      &getPlanetPosition)
            .function("getPlanetVelocity",      &getPlanetVelocity)
            .function("getPlanetMass",          &getPlanetMass)
            .function("getPlanetRadius",        &getPlanetRadius)
            .function("setPlanetPosition",      &setPlanetPosition)
            .function("setPlanetVelocity",      &setPlanetVelocity)
            .function("setPlanetMass",          &setPlanetMass)
            .function("drawTrails",             &drawTrails)
            .function("addOrbital",             &PlanetsUniverse::addOrbital)
            .function("advance",                &PlanetsUniverse::advance)
            .function("centerAll",              &PlanetsUniverse::centerAll)
            .function("deleteAll",              &PlanetsUniverse::deleteAll)
            .function("deleteEscapees",         &PlanetsUniverse::deleteEscapees)
            .function("deleteSelected",         &PlanetsUniverse::deleteSelected)
            .function("generateRandom",         &PlanetsUniverse::generateRandom)
            .function("generateRandomOrbital",  &PlanetsUniverse::generateRandomOrbital)
            .function("getRandomPlanet",        &PlanetsUniverse::getRandomPlanet)
            .function("isEmpty",                &PlanetsUniverse::isEmpty)
            .function("isSelectedValid",        &PlanetsUniverse::isSelectedValid)
            .function("isValid",                &PlanetsUniverse::isValid)
            .function("remove",                 &removePlanet)
            .function("resetSelected",          &PlanetsUniverse::resetSelected)
            .function("size",                   &PlanetsUniverse::size)
            .function("velocityfac",            &getVelocityFac)
            .property("following",              &PlanetsUniverse::following)
            .property("pathLength",             &PlanetsUniverse::pathLength)
            .property("pathRecordDistance",     &PlanetsUniverse::pathRecordDistance)
            .property("selected",               &PlanetsUniverse::selected)
            .property("speed",                  &PlanetsUniverse::simulationSpeed)
            .property("stepsPerFrame",          &PlanetsUniverse::stepsPerFrame)
            ;
}
