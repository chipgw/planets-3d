#include "planetswindow.h"

/* TODO - get opengl pointers properly on Windows */
#define GL_GLEXT_PROTOTYPES
#include <GL/glcorearb.h>

PlanetsWindow::PlanetsWindow(int argc, char *argv[]) : placing(universe), camera(universe) {
    /* TODO - I'm not sure which version of OpenGL I want yet... */
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) == -1) {
        printf("ERROR: Unable to init SDL! \"%s\"", SDL_GetError());
        abort();
    }

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS,  1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,  8);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    windowSDL = SDL_CreateWindow("Planets3D", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED);

    if (!windowSDL){
        printf("ERROR: Unable to create window!");
        abort();
    }

    contextSDL = SDL_GL_CreateContext(windowSDL);

    SDL_GL_SetSwapInterval(0);

    /* TODO - I may want to handle the cursor myself... */
    SDL_ShowCursor(SDL_ENABLE);
}

PlanetsWindow::~PlanetsWindow(){
    SDL_GL_DeleteContext(contextSDL);
    SDL_DestroyWindow(windowSDL);

    SDL_Quit();
}

int PlanetsWindow::run(){
    while(running) {
        doEvents();

        if(placing.step == PlacingInterface::NotPlacing || placing.step == PlacingInterface::Firing){
//            universe.advance(delay);
        }

        paint();

        SDL_GL_SwapWindow(windowSDL);
    }

    return 0;
}

void PlanetsWindow::paint(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* TODO - implement */
}

void PlanetsWindow::toggleFullscreen(){
    fullscreen = !fullscreen;

    if(fullscreen){
        SDL_DisplayMode mode;
        SDL_GetDesktopDisplayMode(0, &mode);
        SDL_SetWindowDisplayMode(windowSDL, &mode);
    }
    SDL_SetWindowFullscreen(windowSDL, SDL_bool(fullscreen));
}

void PlanetsWindow::doEvents(){
    SDL_Event event;
    while(SDL_PollEvent(&event)){
        switch (event.type) {
        case SDL_QUIT:
            onClose();
            break;
        case SDL_KEYDOWN:
            if(event.key.keysym.sym == SDLK_ESCAPE){
                onClose();
            }
            break;
        case SDL_WINDOWEVENT:
            switch(event.window.event){
            case SDL_WINDOWEVENT_RESIZED:
                if(event.window.data1 < 1 || event.window.data2 < 1){
                    SDL_SetWindowSize(windowSDL, std::max(event.window.data1, 1), std::max(event.window.data2, 1));
                    break;
                }
                onResized(event.window.data1, event.window.data2);
                break;
            }
            break;
        case SDL_MOUSEWHEEL:

            break;
        case SDL_MOUSEBUTTONUP:
            if(event.button.button == SDL_BUTTON_LEFT){

            }
            break;
        }
    }
}

void PlanetsWindow::onClose(){
    /* TODO - make warning message and/or menu */
    running = false;
}

void PlanetsWindow::onResized(uint32_t width, uint32_t height){
    windowWidth = width;
    windowHeight = height;

    glViewport(0, 0, width, height);

    camera.resizeViewport(width, height);
}

