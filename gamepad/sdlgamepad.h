#pragma once

#include "types.h"
#include <SDL_gamecontroller.h>
#include <SDL_events.h>
#include <functional>

class PlanetsGamepad {
    PlanetsUniverse& universe;
    Camera& camera;
    PlacingInterface& placing;

    /* The currently active gamepad. */
    SDL_GameController* controller = nullptr;

    /* Are we using the speed trigger?
     * Set to true when trigger leaves deadzone,
     * set to false when speed is locked or it enters the deadzone. */
    bool speedTriggerInUse = false;
    /* Previous value retrieved from the speed trigger.
     * Used when checking if the trigger has left the deadzone. */
    int16_t speedTriggerLast = 0;

public:
    /* Only call if SDL isn't inited already. */
    void initSDL();

    /* Call if event SDL system not handled elsewhere. */
    void pollGamepad();

    /* Call for SDL events in existing event handling loop. */
    void handleEvent(SDL_Event& e);

    void doControllerButtonPress(const Uint8& button);
    void doControllerAxisInput(int32_t delay);

    PlanetsGamepad(PlanetsUniverse& u, Camera& c, PlacingInterface& p) : universe(u), camera(c), placing(p) { }

    inline bool isAttached() const { return controller != nullptr; }

    std::function<void()> closeFunction;

};
