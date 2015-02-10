#pragma once

#ifndef _MSC_VER
#define SDL_MAIN_HANDLED
#endif

#include "planetsuniverse.h"
#include "placinginterface.h"
#include "spheregenerator.h"
#include "grid.h"
#include <SDL.h>

#ifdef PLANETS3D_WITH_GLEW
#include <GL/glew.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/glcorearb.h>
#endif

class PlanetsWindow{
    /* Universe and basic interface classes. */
    PlanetsUniverse universe;
    PlacingInterface placing;
    Camera camera;

    /* SDL handles for the window and GL context */
    SDL_Window *windowSDL;
    SDL_GLContext contextSDL;

    /* The currently active gamepad. */
    SDL_GameController* controller;

    /* Are we using the speed trigger?
     * Set to true when trigger leaves deadzone,
     * set to false when speed is locked or it enters the deadzone. */
    bool speedTriggerInUse;
    /* Previous value retrieved from the speed trigger.
     * Used when checking if the trigger has left the deadzone. */
    int16_t speedTriggerLast;

    /* Is the window currently fullscreen? */
    bool fullscreen;

    /* onClose() sets this to false to stop the primary loop. */
    bool running;

    /* Graphics settings */
    bool drawTrails;

    Grid grid;

    /* Store the window width and height (in pixels) for use with mouse events. */
    uint32_t windowWidth;
    uint32_t windowHeight;

    /* OpenGL vertex attribute handles. */
    const static GLuint vertex, uv;

    /* GL shader and uniform handles for texture shader. */
    GLuint shaderTexture, shaderTexture_vsh, shaderTexture_fsh, shaderTexture_cameraMatrix, shaderTexture_modelMatrix;

    /* GL shader and uniform handles for flat color shader. */
    GLuint shaderColor, shaderColor_vsh, shaderColor_fsh, shaderColor_cameraMatrix, shaderColor_modelMatrix, shaderColor_color;

    /* Generated spheres/circles for rendering */
    const static Sphere<64, 32> highResSphere;
    const static Sphere<32, 16> lowResSphere;
    const static Circle<64> circle;

    /* Currently the only texture used. */
    GLuint planetTexture;

    void updateGrid();

    /* Called to update universe based on SDL events. */
    void doEvents();
    void doKeyPress(const SDL_Keysym &key);
    void doControllerButtonPress(const Uint8 &button);
    void doControllerAxisInput(int64_t delay);

    /* Initialization functions, called by constructor. */
    void initSDL();
    void initGL();
    void initShaders();

    /* Load a texture from a file. */
    GLuint loadTexture(const char* filename);

    /* Render all the stuffs. */
    void paint();

    /* Call this to close the window. */
    void onClose();
    /* Called whenever window gets resized. */
    void onResized(uint32_t width, uint32_t height);

    /* Various planet drawing functions, mostly the same as in the Qt interface. */
    void drawPlanet(const Planet &planet);
    void drawPlanetColor(const Planet &planet, const uint32_t& color);
    void drawPlanetWireframe(const Planet &planet, const uint32_t& color = 0xff00ff00);

    /* Convert a 0xAARRGGBB color value to a float-based vec4. */
    glm::vec4 uintColorToVec4(const uint32_t& color) {
        return glm::vec4((color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff, color >> 24) / 256.0f;
    }

    /* Total amount of frames drawn since window creation. */
    uintmax_t totalFrames;

public:
    /* Create the window, expects command line arguments as passed to a standard main(int argc, char* argv[]) function. */
    PlanetsWindow(int argc, char* argv[]);
    ~PlanetsWindow();

    /* Enter or exit fullscreen mode. */
    void toggleFullscreen();

    /* Runs the event/drawing loop and doesn't return until window is closed. */
    int run();
};
