#include <planetsuniverse.h>
#include <camera.h>
#include <placinginterface.h>
#include <bind.h>
#include <glm/vec2.hpp>
#include <glm/geometric.hpp>
#include <glm/gtx/transform.hpp>

/* Need to wrap these functions because I can't pass Planets without getting them copied for some reason... */
glm::vec3 getPlacingPosition(PlacingInterface& interface) {
    return interface.planet.position;
}
float getPlacingRadius(PlacingInterface& interface) {
    return interface.planet.radius();
}

/* Stupid way of returning two boolean values because pass-by-ref doesn't work... */
struct TwoBool {
    bool b1, b2;
};

TwoBool handleMouseMove(PlacingInterface& interface, glm::ivec2 pos, glm::ivec2 delta, const Camera& cam) {
    TwoBool ret;
    ret.b1 = interface.handleMouseMove(pos, delta, cam, ret.b2);
    return ret;
}

float getArrowLength(PlacingInterface& placing) {
    return glm::length(placing.planet.velocity);
}

glm::mat4 getArrowMat(PlacingInterface& placing) {
    glm::mat4 matrix = glm::translate(placing.planet.position);
    matrix = glm::scale(matrix, glm::vec3(placing.planet.radius()));
    matrix *= placing.rotation;
    return matrix;
}

EMSCRIPTEN_BINDINGS(placinginterface) {
    emscripten::enum_<PlacingInterface::PlacingStep>("PlacingStep")
            .value("NotPlacing",        PlacingInterface::NotPlacing)
            .value("FreePositionXY",    PlacingInterface::FreePositionXY)
            .value("FreePositionZ",     PlacingInterface::FreePositionZ)
            .value("FreeVelocity",      PlacingInterface::FreeVelocity)
            .value("Firing",            PlacingInterface::Firing)
            .value("OrbitalPlane",      PlacingInterface::OrbitalPlane)
            .value("OrbitalPlanet",     PlacingInterface::OrbitalPlanet)
            ;
    emscripten::value_array<TwoBool>("TwoBool")
            .element(&TwoBool::b1)
            .element(&TwoBool::b2)
            ;
    emscripten::class_<PlacingInterface>("PlacingInterface")
            .constructor<PlanetsUniverse&>()
            .function("beginInteractiveCreation",   &PlacingInterface::beginInteractiveCreation)
            .function("beginOrbitalCreation",       &PlacingInterface::beginOrbitalCreation)
            .function("enableFiringMode",           &PlacingInterface::enableFiringMode)
            .function("handleMouseClick",           &PlacingInterface::handleMouseClick)
            .function("handleMouseMove",            &handleMouseMove)
            .function("handleMouseWheel",           &PlacingInterface::handleMouseWheel)
            .function("getPosition",                &getPlacingPosition)
            .function("getRadius",                  &getPlacingRadius)
            .function("getOrbitalCircleMat",        &PlacingInterface::getOrbitalCircleMat)
            .function("getOrbitedCircleMat",        &PlacingInterface::getOrbitedCircleMat)
            .function("getArrowLength",             &getArrowLength)
            .function("getArrowMat",                &getArrowMat)
            .property("step",                       &PlacingInterface::step)
            ;
}
