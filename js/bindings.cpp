#include <planetsuniverse.h>
#include <camera.h>
#include <planet.h>
#include <bind.h>
#include <glm/vec2.hpp>

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

EMSCRIPTEN_BINDINGS(planets_universe) {
    emscripten::class_<PlanetsUniverse>("PlanetsUniverse")
            .constructor()
            .function("addPlanet", &createPlanet)
            .function("addPlanet", &createPlanetPos)
            .function("addPlanet", &createPlanetPosVel)
            .function("addPlanet", &createPlanetAll)
            .function("addOrbital",             &PlanetsUniverse::addOrbital)
            .function("advance",                &PlanetsUniverse::advance)
            .function("centerAll",              &PlanetsUniverse::centerAll)
            .function("deleteAll",              &PlanetsUniverse::deleteAll)
            .function("deleteEscapees",         &PlanetsUniverse::deleteEscapees)
            .function("deleteSelected",         &PlanetsUniverse::deleteSelected)
            .function("generateRandom",         &PlanetsUniverse::generateRandom)
            .function("generateRandomOrbital",  &PlanetsUniverse::generateRandomOrbital)
            .function("getRandomPlanet",        &PlanetsUniverse::getRandomPlanet)
            .function("getSelected",            &PlanetsUniverse::getSelected)
            .function("isEmpty",                &PlanetsUniverse::isEmpty)
            .function("isSelectedValid",        &PlanetsUniverse::isSelectedValid)
            .function("isValid",                &PlanetsUniverse::isValid)
            .function("nextKey",                &PlanetsUniverse::nextKey)
            .function("prevKey",                &PlanetsUniverse::prevKey)
            .function("remove",                 &PlanetsUniverse::remove)
            .function("resetSelected",          &PlanetsUniverse::resetSelected)
            .function("size",                   &PlanetsUniverse::size)
            .function("get",                    &PlanetsUniverse::operator [])
            .property("following",              &PlanetsUniverse::following)
            .property("pathLength",             &PlanetsUniverse::pathLength)
            .property("pathRecordDistance",     &PlanetsUniverse::pathRecordDistance)
            .property("selected",               &PlanetsUniverse::selected)
            .property("speed",                  &PlanetsUniverse::simspeed)
            .property("stepsPerFrame",          &PlanetsUniverse::stepsPerFrame)
            .property("velocityfac",            &PlanetsUniverse::velocityfac)
            ;
}

EMSCRIPTEN_BINDINGS(planet) {
    emscripten::class_<Planet>("Planet")
            .property("mass", &Planet::mass, &Planet::setMass)
            .property("position", &Planet::position)
            .property("velocity", &Planet::velocity)
            .property("radius", &Planet::radius)
            ;
}

EMSCRIPTEN_BINDINGS(vec3) {
    /* TODO - Do we need functions for vectors? */
    emscripten::value_array<glm::vec3>("vec3")
            .element(&glm::vec3::x)
            .element(&glm::vec3::y)
            .element(&glm::vec3::z)
            ;
}

EMSCRIPTEN_BINDINGS(ivec2) {
    emscripten::value_array<glm::ivec2>("ivec2")
            .element(&glm::ivec2::x)
            .element(&glm::ivec2::y)
            ;
}

template<int x, int y> float getMatElement(const glm::mat4& mat) {
    return mat[x][y];
}

template<int x, int y> void setMatElement(glm::mat4& mat, float val) {
    mat[x][y] = val;
}

glm::mat4 getIdentityMatrix() { return glm::mat4(); }

/* TODO - This seems like it wouldn't be the best way to do this,
 * plus I don't know if matrix manipulation functions will need to be called in JS... */
EMSCRIPTEN_BINDINGS(mat4) {
    emscripten::value_array<glm::mat4>("mat4")
            .element(&getMatElement<0, 0>, &setMatElement<0, 0>)
            .element(&getMatElement<0, 1>, &setMatElement<0, 1>)
            .element(&getMatElement<0, 2>, &setMatElement<0, 2>)
            .element(&getMatElement<0, 3>, &setMatElement<0, 3>)
            .element(&getMatElement<1, 0>, &setMatElement<1, 0>)
            .element(&getMatElement<1, 1>, &setMatElement<1, 1>)
            .element(&getMatElement<1, 2>, &setMatElement<1, 2>)
            .element(&getMatElement<1, 3>, &setMatElement<1, 3>)
            .element(&getMatElement<2, 0>, &setMatElement<2, 0>)
            .element(&getMatElement<2, 1>, &setMatElement<2, 1>)
            .element(&getMatElement<2, 2>, &setMatElement<2, 2>)
            .element(&getMatElement<2, 3>, &setMatElement<2, 3>)
            .element(&getMatElement<3, 0>, &setMatElement<3, 0>)
            .element(&getMatElement<3, 1>, &setMatElement<3, 1>)
            .element(&getMatElement<3, 2>, &setMatElement<3, 2>)
            .element(&getMatElement<3, 3>, &setMatElement<3, 3>)
            ;
    emscripten::function("getIdentityMatrix", &getIdentityMatrix);
}

EMSCRIPTEN_BINDINGS(camera) {
    emscripten::class_<Camera>("Camera")
            .constructor<PlanetsUniverse&>()
            .function("bound",                  &Camera::bound)
            .function("clearFollow",            &Camera::clearFollow)
            .function("followNext",             &Camera::followNext)
            .function("followPrevious",         &Camera::followPrevious)
            .function("followPlainAverage",     &Camera::followPlainAverage)
            .function("followWeightedAverage",  &Camera::followWeightedAverage)
            .function("followSelection",        &Camera::followSelection)
            .function("reset",                  &Camera::reset)
            .function("resizeViewport",         &Camera::resizeViewport)
            .function("selectUnder",            &Camera::selectUnder)
            .function("setup",                  &Camera::setup)
            .property("position",               &Camera::position)
            .property("distance",               &Camera::distance)
            .property("xrotation",              &Camera::xrotation)
            .property("zrotation",              &Camera::zrotation)
            ;
}
