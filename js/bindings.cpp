#include <planetsuniverse.h>
#include <camera.h>
#include <planet.h>
#include <bind.h>

EMSCRIPTEN_BINDINGS(planets_universe) {
    emscripten::class_<PlanetsUniverse>("PlanetsUniverse")
            .constructor()
            .function("addOrbital",             &PlanetsUniverse::addOrbital)
            .function("addPlanet",              &PlanetsUniverse::addPlanet)
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
            .constructor()
            .constructor<glm::vec3>()
            .constructor<glm::vec3, glm::vec3>()
            .constructor<glm::vec3, glm::vec3, float>()
            .property("mass", &Planet::mass, &Planet::setMass)
            .property("position", &Planet::position)
            .property("velocity", &Planet::velocity)
            ;
}

EMSCRIPTEN_BINDINGS(vec3) {
    /* Use JS convention of capital letter starting types. */
    emscripten::class_<glm::vec3>("Vec3")
            .constructor()
            .constructor<float, float, float>()
            .property("x", &glm::vec3::x)
            .property("y", &glm::vec3::y)
            .property("z", &glm::vec3::z)
            ;
}

EMSCRIPTEN_BINDINGS(mat4) {
    /* Use JS convention of capital letter starting types. */
    emscripten::class_<glm::mat4>("Mat4")
            .constructor()
            ;
}

EMSCRIPTEN_BINDINGS(camera) {
    emscripten::class_<Camera>("Camera")
            .constructor<PlanetsUniverse&>()
            .function("setup",        &Camera::setup)
            .property("position",     &Camera::position)
            .property("distance",     &Camera::distance)
            .property("xrotation",    &Camera::xrotation)
            .property("zrotation",    &Camera::zrotation)
            ;
}
