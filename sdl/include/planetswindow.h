#ifndef PLANETSWINDOW_H
#define PLANETSWINDOW_H

#ifndef _MSC_VER
#define SDL_MAIN_HANDLED
#endif

#include "planetsuniverse.h"
#include "placinginterface.h"
#include "spheregenerator.h"
#include <SDL.h>

#ifdef PLANETS3D_WITH_GLEW
#include <GL/glew.h>
#else
/* TODO - get opengl pointers properly on Windows */
#define GL_GLEXT_PROTOTYPES
#include <GL/glcorearb.h>
#endif

class PlanetsWindow{
    PlanetsUniverse universe;
    PlacingInterface placing;
    Camera camera;

    SDL_Window *windowSDL;
    SDL_GLContext contextSDL;

    bool fullscreen;
    bool running;

    bool drawTrails;

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

    void initSDL();
    void initGL();
    void initShaders();

    GLuint loadTexture(const char* filename);

    void paint();

    void onClose();
    void onResized(uint32_t width, uint32_t height);

    void drawPlanet(const Planet &planet);
    void drawPlanetColor(const Planet &planet, const uint32_t& color);
    void drawPlanetWireframe(const Planet &planet, const uint32_t& color = 0xff00ff00);

    glm::vec4 uintColorToVec4(const uint32_t& color) { return glm::vec4((color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff, color >> 24) / 256.0f; }

    uintmax_t totalFrames;

public:
    PlanetsWindow(int argc, char* argv[]);
    ~PlanetsWindow();

    void toggleFullscreen();

    int run();
};

#endif // PLANETSWINDOW_H
