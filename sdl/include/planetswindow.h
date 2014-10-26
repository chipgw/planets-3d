#ifndef PLANETSWINDOW_H
#define PLANETSWINDOW_H

#ifndef _MSC_VER
#define SDL_MAIN_HANDLED
#endif

#include "planetsuniverse.h"
#include "placinginterface.h"
#include <SDL.h>

class PlanetsWindow{
    PlanetsUniverse universe;
    PlacingInterface placing;
    Camera camera;

    SDL_Window *windowSDL;
    SDL_GLContext contextSDL;

    bool fullscreen;
    bool running;

    uint32_t windowWidth;
    uint32_t windowHeight;

    void doEvents();

public:
    PlanetsWindow(int argc, char* argv[]);
    ~PlanetsWindow();

    void toggleFullscreen();

    int run();
    void paint();

    void onClose();
    void onResized(uint32_t width, uint32_t height);


    void drawPlanet(const Planet &planet);
};

#endif // PLANETSWINDOW_H
