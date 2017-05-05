#include <sdlgamepad.h>
#include <emscripten/bind.h>
#include <planetsuniverse.h>
#include <camera.h>
#include <placinginterface.h>

EMSCRIPTEN_BINDINGS(gamepad) {
    emscripten::class_<PlanetsGamepad>("Gamepad")
            .constructor<PlanetsUniverse&, Camera&, PlacingInterface&>()
            .function("init",           &PlanetsGamepad::initSDL)
            .function("doAxisInput",    &PlanetsGamepad::doControllerAxisInput)
            .function("pollInput",      &PlanetsGamepad::pollGamepad)
            .property("attached",       &PlanetsGamepad::isAttached)
            ;
}
