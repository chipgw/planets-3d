#include "sdlgamepad.h"
#include "camera.h"
#include "planetsuniverse.h"
#include "placinginterface.h"
#include <SDL.h>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <cstdio>

const int16_t triggerDeadzone = 16;

void PlanetsGamepad::initSDL() {
    if (SDL_Init(SDL_INIT_GAMECONTROLLER) == -1)
        printf("ERROR: Unable to init SDL! \"%s\"", SDL_GetError());

    /* We need events from the controllers, as we don't poll buttons. */
    SDL_GameControllerEventState(SDL_ENABLE);
}

void PlanetsGamepad::pollGamepad() {
    SDL_Event event;

    while(SDL_PollEvent(&event))
        handleEvent(event);
}

void PlanetsGamepad::handleEvent(SDL_Event& e) {
    switch (e.type) {
    case SDL_CONTROLLERDEVICEADDED:
        /* This will allow us to recieve events from the controller. */
        SDL_GameControllerOpen(e.cdevice.which);
        break;
    case SDL_CONTROLLERBUTTONUP:
        /* If we haven't picked a controller yet or the guide button is pressed, use this one. */
        if (controller == nullptr || e.cbutton.button == SDL_CONTROLLER_BUTTON_GUIDE)
            controller = SDL_GameControllerOpen(e.cbutton.which);

        /* Ignore events from other controllers. */
        if (e.cbutton.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller)))
            doControllerButtonPress(e.cbutton.button);

        break;
    }
}

void PlanetsGamepad::doControllerButtonPress(const uint8_t& button) {
    switch(button) {
    case SDL_CONTROLLER_BUTTON_BACK:
        if (closeFunction != nullptr)
            closeFunction();
        break;
    case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
        camera.reset();
        break;
    case SDL_CONTROLLER_BUTTON_LEFTSTICK:
        /* Left stick resets camera position without touching angle or zoom. */
        camera.position = glm::vec3();
        break;
    case SDL_CONTROLLER_BUTTON_A:
        /* TODO - there should probably be a seperate function for this... */
        if (!placing.handleMouseClick(camera.getCenterScreen(), camera))
            camera.selectUnder(camera.getCenterScreen());
        break;
    case SDL_CONTROLLER_BUTTON_X:
        universe.deleteSelected();
        break;
    case SDL_CONTROLLER_BUTTON_Y:
        /* Automatically select orbital or normal interactive placement based on selection. */
        if (universe.isSelectedValid())
            placing.beginOrbitalCreation();
        else
            placing.beginInteractiveCreation();
        break;
    case SDL_CONTROLLER_BUTTON_B:
        /* If trigger is not being held down pause/resume. */
        if (speedTriggerLast < triggerDeadzone)
            universe.simspeed = universe.simspeed <= 0.0f ? 1.0f : 0.0f;

        /* If the trigger is being held down lock to current speed. */
        speedTriggerInUse = false;
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
        camera.followPrevious();
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
        camera.followNext();
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
        camera.clearFollow();
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_UP:
        /* Toggle between the two types of average follow. */
        if (camera.followingState == Camera::WeightedAverage)
            camera.followPlainAverage();
        else
            camera.followWeightedAverage();
        break;
    }
}

void PlanetsGamepad::doControllerAxisInput(int32_t delay) {
    /* TODO - lots of magic numbers in this function... */
    if (controller != nullptr) {
        /* We only need it as a float, might as well not call and convert it repeatedly... */
        const float int16_max = std::numeric_limits<Sint16>::max();
        /* As we compare to length2 we need to use the square of Sint16's maximum value. */
        const float stickDeadzone = 0.1f * int16_max * int16_max;

        /* Multiply analog stick value by this to map from int16_max to delay converted to seconds. */
        float stickFac = delay * 1.0e-6f / int16_max;

        glm::vec2 right(SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX),
                        SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY));

        if (glm::length2(right) > stickDeadzone) {
            /* Map values to the proper range. */
            right *= stickFac;

            bool rsMod = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) != 0;

            if (rsMod) {
                camera.distance += right.y * camera.distance;
            } else {
                camera.xrotation += right.y;
                camera.zrotation += right.x;
            }
        }

        camera.bound();

        glm::vec2 left(SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX),
                       SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY));

        if (glm::length2(left) > stickDeadzone) {
            /* Map values to the proper range. */
            left *= stickFac;

            bool lsMod = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER) != 0;

            if (!placing.handleAnalogStick(left, lsMod, camera)) {
                /* If the camera is following something stick is used only for zoom. */
                if (lsMod || camera.followingState != Camera::FollowNone)
                    camera.distance += left.y * camera.distance;
                else
                    camera.position += glm::vec3(glm::vec4(left.x, 0.0f, -left.y, 0.0f) * camera.camera) * camera.distance;
            }
        }

        int16_t speedTriggerCurrent = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);

        /* If the trigger has gone from disengaged (< deadzone) to engaged (> deadzone) we enable using it as speed input. */
        if (speedTriggerInUse || (speedTriggerCurrent > triggerDeadzone && speedTriggerLast <= triggerDeadzone)) {
            universe.simspeed = float(speedTriggerCurrent * 8) / int16_max;
            universe.simspeed *= universe.simspeed;

            speedTriggerInUse = true;
        }

        speedTriggerLast = speedTriggerCurrent;
    }
}
