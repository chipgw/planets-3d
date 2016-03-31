#include <planetsuniverse.h>
#include <camera.h>
#include <bind.h>

EMSCRIPTEN_BINDINGS(planets_universe) {
  emscripten::class_<PlanetsUniverse>("PlanetsUniverse")
          .constructor()
          .function("addOrbital",       &PlanetsUniverse::addOrbital)
          .function("addPlanet",        &PlanetsUniverse::addPlanet)
          .function("advance",          &PlanetsUniverse::advance)
          .function("deleteAll",        &PlanetsUniverse::deleteAll)
          .function("deleteEscapees",   &PlanetsUniverse::deleteEscapees)
          .function("deleteSelected",   &PlanetsUniverse::deleteSelected)
          .function("generateRandom",   &PlanetsUniverse::generateRandom)
          .function("getRandomPlanet",  &PlanetsUniverse::getRandomPlanet)
          .function("getSelected",      &PlanetsUniverse::getSelected)
          .function("isEmpty",          &PlanetsUniverse::isEmpty)
          .function("isSelectedValid",  &PlanetsUniverse::isSelectedValid)
          .function("isValid",          &PlanetsUniverse::isValid)
          .function("nextKey",          &PlanetsUniverse::nextKey)
          .function("prevKey",          &PlanetsUniverse::prevKey)
          .function("remove",           &PlanetsUniverse::remove)
          .function("resetSelected",    &PlanetsUniverse::resetSelected)
          .property("following",        &PlanetsUniverse::following)
          .property("pathLength",       &PlanetsUniverse::pathLength)
          .property("pathRecordDist",   &PlanetsUniverse::pathRecordDistance)
          .property("selected",         &PlanetsUniverse::selected)
          .property("simspeed",         &PlanetsUniverse::simspeed)
          .property("stepsPerFrame",    &PlanetsUniverse::stepsPerFrame)
          .property("velocityfac",      &PlanetsUniverse::velocityfac)
    ;
}
