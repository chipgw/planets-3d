#ifndef PLANETSWINDOW_H
#define PLANETSWINDOW_H

#ifndef _MSC_VER
#define SDL_MAIN_HANDLED
#endif

#include "planetsuniverse.h"
#include "placinginterface.h"
#include "spheregenerator.h"
#include <SDL.h>

/* TODO - get opengl pointers properly on Windows */
#define GL_GLEXT_PROTOTYPES
#include <GL/glcorearb.h>

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

    const static GLuint vertex, uv;

    GLuint shaderTexture, shaderTexture_vsh, shaderTexture_fsh, shaderTexture_cameraMatrix, shaderTexture_modelMatrix;

    GLuint shaderColor, shaderColor_vsh, shaderColor_fsh, shaderColor_cameraMatrix, shaderColor_modelMatrix, shaderColor_color;

    const static Sphere<64, 32> highResSphere;
    const static Sphere<32, 16> lowResSphere;
    const static Circle<64> circle;

    GLuint planetTexture;

    void doEvents();

    bool initSDL();
    bool initGL();
    bool initShaders();

    GLuint loadTexture(const char* filename);

    void paint();

    void onClose();
    void onResized(uint32_t width, uint32_t height);

    void drawPlanet(const Planet &planet);

    uintmax_t totalFrames;

public:
    PlanetsWindow(int argc, char* argv[]);
    ~PlanetsWindow();

    void toggleFullscreen();

    int run();
};

#endif // PLANETSWINDOW_H
