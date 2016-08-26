#pragma once

#ifndef _MSC_VER
#define SDL_MAIN_HANDLED
#endif

#include "planetsuniverse.h"
#include "placinginterface.h"
#include "grid.h"
#include "camera.h"
#include "sdlgamepad.h"
#include <SDL.h>

class PlanetsWindow{
    /* Universe and basic interface classes. */
    PlanetsUniverse universe;
    PlacingInterface placing;
    Camera camera;

    /* Store the current speed in here when pausing. */
    float pauseSpeed = 1.0f;

    /* SDL handles for the window and GL context */
    SDL_Window* windowSDL;
    SDL_GLContext contextSDL;

    PlanetsGamepad gamepad;

    /* Is the window currently fullscreen? */
    bool fullscreen = false;

    /* onClose() sets this to false to stop the primary loop. */
    bool running = false;

    /* Graphics settings */
    bool drawTrails = false;
    bool drawPlanarCircles = false;

    Grid grid;

    /* Store the window width and height (in pixels) for use with mouse events. */
    glm::ivec2 windowSize;

    /* GL shader and uniform handles for texture shader. */
    unsigned int shaderTexture, shaderTexture_cameraMatrix, shaderTexture_viewMatrix, shaderTexture_modelMatrix, shaderTexture_lightDir;

    /* GL shader and uniform handles for flat color shader. */
    unsigned int shaderColor, shaderColor_cameraMatrix, shaderColor_modelMatrix, shaderColor_color;

    /* GL shader and uniform handles for dear imgui. */
    unsigned int shaderUI, shaderUI_matrix;

    /* The diffuse and normalmap textures for planets. */
    unsigned int planetTexture_diff, planetTexture_nrm;

    unsigned int highResVBO, highResTriIBO, highResTriCount;
    unsigned int lowResVBO, lowResLineIBO, lowResLineCount;
    unsigned int circleVBO, circleLineIBO, circleLineCount;

    void updateGrid();

    /* Called to update universe based on SDL events. */
    void doEvents();
    bool doKeyPress(const SDL_Keysym& key);

    /* Initialization functions, called by constructor. */
    void initSDL();
    void initGL();
    void initShaders();
    void initBuffers();
    void initUI();

    /* Load a texture from a file. */
    unsigned int loadTexture(SDL_RWops* io);

    /* Render all the stuffs. */
    void paint();
    void paintUI(const float delay);

    /* Call this to close the window. */
    void onClose();

    /* Call to show a confirmation message to delete planets. */
    void newUniverse();

    /* Called whenever window gets resized. */
    void onResized(uint32_t width, uint32_t height);

    /* Draw a wireframe planet. Expects low res sphere VBO & IBO to be bound. */
    void drawPlanetWireframe(const Planet& planet, const uint32_t& color = 0xff00ff00);

    /* Convert a 0xAARRGGBB color value to a float-based vec4. */
    glm::vec4 uintColorToVec4(const uint32_t& color) {
        return glm::vec4((color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff, color >> 24) / 256.0f;
    }

    /* Total amount of frames drawn since window creation. */
    uintmax_t totalFrames = 0;

    /* Keep track of the delayof the last fer frames for graphing. */
    std::array<float, 32> frameTimes;
    size_t frameTimeOffset = 0;

public:
    /* Create the window, expects command line arguments as passed to a standard main(int argc, char* argv[]) function. */
    PlanetsWindow(int argc, char* argv[]);
    ~PlanetsWindow();

    /* Enter or exit fullscreen mode. */
    void toggleFullscreen();

    /* Runs the event/drawing loop and doesn't return until window is closed. */
    void run();
};
