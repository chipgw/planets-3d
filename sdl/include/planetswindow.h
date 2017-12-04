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
#include <array>

class PlanetsWindow {
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
    float drawScale = 1.0f;

    Grid grid;

    /* Store the window width and height (in pixels) for use with mouse events. */
    glm::ivec2 windowSize;

    /* GL shader and uniform handles for texture shader. */
    unsigned int shaderTexture, shaderTexture_cameraMatrix, shaderTexture_viewMatrix, shaderTexture_modelMatrix, shaderTexture_lightDir, shaderTexture_material;

    /* GL shader and uniform handles for flat color shader. */
    unsigned int shaderColor, shaderColor_cameraMatrix, shaderColor_modelMatrix, shaderColor_color;

    /* GL shader and uniform handles for dear imgui. */
    unsigned int shaderUI, shaderUI_matrix;

    /* The handles for planet texture arrays. */
    unsigned int planetTextures_diff;
    unsigned int planetTextures_nrm;
    unsigned int planetTextures_height;

    unsigned int highResVBO, highResTriIBO, highResTriCount;
    unsigned int lowResVBO, lowResLineIBO, lowResLineCount;
    unsigned int circleVBO, circleLineIBO, circleLineCount;
    unsigned int gridVBO;

    void updateGrid();

    /* Called to update universe based on SDL events. */
    void doEvents();
    void doKeyPress(const SDL_Keysym& key);

    /* Initialization functions, called by constructor. */
    void initSDL();
    void initGL();
    void initShaders();
    void initBuffers();
    void initUI();

#ifdef PLANETS3D_WITH_NFD
    void openFile();
    void appendFile();
    void saveFile();
#endif

    /* Load textures into a 2d texture array (assumes textures are in "texture/" relative to program). */
    unsigned int loadTextures(std::vector<std::string> files);

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
    void drawPlanetWireframe(const Planet& planet, const glm::vec4& color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));

    /* Total amount of frames drawn since window creation. */
    uintmax_t totalFrames = 0;

    /* Keep track of the delay of the last 80 frames for graphing. */
    std::array<float, 80> frameTimes;
    size_t frameTimeOffset = 0;

    /* UI variables. */
    bool showPlanetGenWindow = false;
    bool showSpeedWindow = false;
    bool showViewSettingsWindow = false;
    bool showInfoWindow = false;
    bool showFiringWindow = false;
    bool showAboutWindow = false;

#ifndef NDEBUG
    bool showTestWindow = false;
#endif

    bool planetGenOrbital = false;
    int planetGenAmount = 10;
    float planetGenMaxPos = 1.0e3f;
    float planetGenMaxSpeed = 1.0f;
    float planetGenMaxMass = 200.0f;

    std::array<SDL_Cursor*, 7> cursors;

    std::string glInfo;

public:
    /* Create the window, expects command line arguments as passed to a standard main(int argc, char* argv[]) function. */
    PlanetsWindow(int argc, char* argv[]);
    ~PlanetsWindow();

    /* Enter or exit fullscreen mode. */
    void toggleFullscreen();

    /* Runs the event/drawing loop and doesn't return until window is closed. */
    void run();
};
