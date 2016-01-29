#include "planetswindow.h"
#include "shaders.h"
#include "spheregenerator.h"

#include <algorithm>
#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>
#include <SDL_image.h>

#ifdef PLANETS3D_WITH_GLEW
#include <GL/glew.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/glcorearb.h>
#endif

const int16_t triggerDeadzone = 16;

PlanetsWindow::PlanetsWindow(int argc, char *argv[]) : placing(universe), camera(universe) {
    initSDL();
    initGL();

    /* Try loading from the command line. Ignore invalid files and break on the first successful file. */
    std::string err;
    for (int i = 0; i < argc; ++i)
        if(universe.load(argv[i], err)) break;
}

PlanetsWindow::~PlanetsWindow() {
    glDeleteBuffers(1, &highResVBO);
    glDeleteBuffers(1, &highResTriIBO);
    glDeleteBuffers(1, &lowResVBO);
    glDeleteBuffers(1, &lowResLineIBO);
    glDeleteBuffers(1, &circleVBO);
    glDeleteBuffers(1, &circleLineIBO);
    glDeleteProgram(shaderTexture);
    glDeleteProgram(shaderColor);

    SDL_GL_DeleteContext(contextSDL);
    SDL_DestroyWindow(windowSDL);

    SDL_Quit();
}

void PlanetsWindow::initSDL() {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) == -1) {
        printf("ERROR: Unable to init SDL! \"%s\"", SDL_GetError());
        abort();
    }

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS,  1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,  8);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    windowSDL = SDL_CreateWindow("Planets3D", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600,
                                 SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED);

    if (!windowSDL) {
        printf("ERROR: Unable to create window!");
        abort();
    }

    contextSDL = SDL_GL_CreateContext(windowSDL);

    SDL_GL_SetSwapInterval(0);

    /* TODO - I may want to handle the cursor myself... */
    SDL_ShowCursor(SDL_ENABLE);

    SDL_GameControllerEventState(SDL_ENABLE);
}

void PlanetsWindow::initGL() {
    SDL_GL_MakeCurrent(windowSDL, contextSDL);

#ifdef PLANETS3D_WITH_GLEW
    GLuint glewstatus = glewInit();
    if (glewstatus != GLEW_OK) {
        printf("GLEW ERROR: %s", glewGetErrorString(glewstatus));
        abort();
    }
    if (!GLEW_VERSION_4_0) {
        printf("WARNING: OpenGL 4.0 support NOT detected, things probably won't work.");
    }
#endif

    initShaders();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepthf(1.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#ifdef GL_LINE_SMOOTH
    glEnable(GL_LINE_SMOOTH);
#endif

    glEnableVertexAttribArray(vertex);

    planetTexture = loadTexture("planet.png");

    initBuffers();
}

unsigned int PlanetsWindow::loadTexture(const char* filename) {
    SDL_Surface* image = IMG_Load(filename);

    if (image == nullptr) {
        /* Qt Creator's output panel doesn't seem to like the '\r' character for some reason... */
        std::string err(IMG_GetError());
        err.erase(err.find('\r'));

        printf("Failed to load texture! Error: %s\n", err.c_str());

        return 0;
    }

    /* We need the format to be R8G8B8A8 for upload to GPU. */
    SDL_Surface* converted = SDL_ConvertSurface(image, SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888), 0);
    SDL_FreeSurface(image);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, converted->w, converted->h, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, converted->pixels);

    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    SDL_FreeSurface(converted);

    return texture;
}

GLuint compileShader(const char *source, GLenum shaderType) {
    GLuint shader = glCreateShader(shaderType);

    glShaderSource(shader, 1, (const GLchar**)&source, 0);

    glCompileShader(shader);

    int isCompiled, maxLength;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

    if (maxLength > 1) {
        char *infoLog = new char[maxLength];

        glGetShaderInfoLog(shader, maxLength, &maxLength, infoLog);

        if (isCompiled == GL_FALSE) {
            printf("ERROR: failed to compile shader!\n\tLog: %s", infoLog);
            delete[] infoLog;

            return 0;
        } else {
            printf("INFO: succesfully compiled shader.\n\tLog: %s", infoLog);
            delete[] infoLog;
        }
    } else if (isCompiled == GL_FALSE) {
        printf("ERROR: failed to compile shader! No log availible.\n");
        return 0;
    }
    return shader;
}

int linkShaderProgram(GLuint vsh, GLuint fsh) {
    GLuint program = glCreateProgram();

    glAttachShader(program, vsh);
    glAttachShader(program, fsh);

    glLinkProgram(program);

    int isLinked, maxLength;
    glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

    if (maxLength > 1) {
        char *infoLog = new char[maxLength];

        glGetProgramInfoLog(program, maxLength, &maxLength, infoLog);

        if (isLinked == GL_FALSE) {
            printf("ERROR: failed to link shader program!\n\tLog: %s", infoLog);
            delete[] infoLog;

            return 0;
        } else {
            printf("INFO: succesfully linked shader.\n\tLog: %s", infoLog);
            delete[] infoLog;
        }
    } else if (isLinked == GL_FALSE) {
        printf("ERROR: failed to link shader program! No log availible.\n");
        return 0;
    }

    return program;
}

void PlanetsWindow::initShaders() {
    /* Compile the flat color shader included in shaders.h as const char*. */
    GLuint shaderColor_vsh = compileShader(color_vertex_src,     GL_VERTEX_SHADER);
    GLuint shaderColor_fsh = compileShader(color_fragment_src,   GL_FRAGMENT_SHADER);
    shaderColor = linkShaderProgram(shaderColor_vsh, shaderColor_fsh);

    /* These aren't needed anymore... */
    glDeleteShader(shaderColor_vsh);
    glDeleteShader(shaderColor_fsh);

    /* Get the uniform locations from color shader. */
    glUseProgram(shaderColor);
    shaderColor_color           = glGetUniformLocation(shaderColor, "color");
    shaderColor_cameraMatrix    = glGetUniformLocation(shaderColor, "cameraMatrix");
    shaderColor_modelMatrix     = glGetUniformLocation(shaderColor, "modelMatrix");

    glBindAttribLocation(shaderColor, vertex, "vertex");

    /* Compile the textured shader included in shaders.h as const char*. */
    GLuint shaderTexture_vsh = compileShader(texture_vertex_src,   GL_VERTEX_SHADER);
    GLuint shaderTexture_fsh = compileShader(texture_fragment_src, GL_FRAGMENT_SHADER);
    shaderTexture = linkShaderProgram(shaderTexture_vsh, shaderTexture_fsh);

    /* These aren't needed anymore... */
    glDeleteShader(shaderTexture_vsh);
    glDeleteShader(shaderTexture_fsh);

    /* Get the uniform locations from the texture shader. */
    glUseProgram(shaderTexture);
    shaderTexture_cameraMatrix  = glGetUniformLocation(shaderTexture, "cameraMatrix");
    shaderTexture_modelMatrix   = glGetUniformLocation(shaderTexture, "modelMatrix");

    glBindAttribLocation(shaderTexture, vertex, "vertex");
    glBindAttribLocation(shaderTexture, uv,     "uv");
}

void PlanetsWindow::initBuffers() {
    Sphere<64, 32> highResSphere;
    Sphere<32, 16> lowResSphere;
    Circle<64> circle;

    glGenBuffers(1, &highResVBO);
    glBindBuffer(GL_ARRAY_BUFFER, highResVBO);
    glBufferData(GL_ARRAY_BUFFER, highResSphere.vertexCount * sizeof(Vertex), highResSphere.verts, GL_STATIC_DRAW);

    glGenBuffers(1, &highResTriIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, highResTriIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, highResSphere.triangleCount * sizeof(uint32_t), highResSphere.triangles, GL_STATIC_DRAW);

    glGenBuffers(1, &lowResVBO);
    glBindBuffer(GL_ARRAY_BUFFER, lowResVBO);
    glBufferData(GL_ARRAY_BUFFER, lowResSphere.vertexCount * sizeof(Vertex), lowResSphere.verts, GL_STATIC_DRAW);

    glGenBuffers(1, &lowResLineIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lowResLineIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, lowResSphere.lineCount * sizeof(uint32_t), lowResSphere.lines, GL_STATIC_DRAW);

    glGenBuffers(1, &circleVBO);
    glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
    glBufferData(GL_ARRAY_BUFFER, circle.vertexCount * sizeof(Vertex), circle.verts, GL_STATIC_DRAW);

    glGenBuffers(1, &circleLineIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, circleLineIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, circle.lineCount * sizeof(uint32_t), circle.lines, GL_STATIC_DRAW);

    highResTriCount = highResSphere.triangleCount;
    lowResLineCount = lowResSphere.lineCount;
    circleLineCount = circle.lineCount;
}

int PlanetsWindow::run() {
    running = true;

    typedef std::chrono::high_resolution_clock clock;
    using std::chrono::duration_cast;
    using std::chrono::microseconds;

    clock::time_point start_time = clock::now();
    clock::time_point last_time = start_time;

    while (running) {
        /* Figure out how long the last frame took to render & display, in microseconds. */
        clock::time_point current = clock::now();
        int delay = duration_cast<microseconds>(current - last_time).count();
        last_time = current;

        if (delay != 0)
            /* Put a bunch of information into the title. */
            SDL_SetWindowTitle(windowSDL, ("Planets3D  [" + std::to_string(1000000 / delay) + "fps, " + std::to_string(delay / 1000) + "ms " + std::to_string(universe.size()) + " planet(s), " +
                                           std::to_string(universe.simspeed) + "x speed, " + std::to_string(universe.stepsPerFrame) + " step(s), " +  std::to_string(universe.pathLength) + " path length]").c_str());

        /* Don't do delays larger than a second. */
        delay = std::min(delay, 1000000);

        doControllerAxisInput(delay);
        doEvents();

        /* Don't advance if we're placing. */
        if (placing.step == PlacingInterface::NotPlacing || placing.step == PlacingInterface::Firing)
            universe.advance(float(delay));

        paint();

        SDL_GL_SwapWindow(windowSDL);

        ++totalFrames;

        /* So anything that was written to console gets written. */
        fflush(stdout);
    }

    /* Output stats to the console. */
    clock::time_point current = clock::now();
    printf("Total Time: %f seconds.\n", (duration_cast<microseconds>(current - start_time).count() * 1.0e-6f));
    printf("Total Frames: %i.\n", totalFrames);
    printf("Average Draw Time: %fms.\n", (duration_cast<microseconds>(current - start_time).count() * 1.0e-3f) / float(totalFrames));
    printf("Average Framerate: %f fps.\n", (1.0f / (duration_cast<microseconds>(current - start_time).count() / float(totalFrames))) * 1.0e6f);

    /* TODO - There might be some places where we should return something other than 0. (i.e. on a fatal error.)
     * If not why should this return anything? */
    return 0;
}

void PlanetsWindow::paint() {
    /* Make sure we're using the right GL context and clear the window. */
    SDL_GL_MakeCurrent(windowSDL, contextSDL);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    camera.setup();

    /* We only use the texture shader and uv's for drawing the textured planets. */
    glUseProgram(shaderTexture);
    glEnableVertexAttribArray(uv);

    glUniformMatrix4fv(shaderTexture_cameraMatrix, 1, GL_FALSE, glm::value_ptr(camera.camera));
    glUniformMatrix4fv(shaderTexture_modelMatrix, 1, GL_FALSE, glm::value_ptr(glm::mat4()));

    glBindBuffer(GL_ARRAY_BUFFER, highResVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, highResTriIBO);

    for (const auto& i : universe)
        drawPlanet(i.second);

    /* Now the texture shader and uv's don't get used until next frame. */
    glDisableVertexAttribArray(uv);
    glUseProgram(shaderColor);

    glUniformMatrix4fv(shaderColor_cameraMatrix, 1, GL_FALSE, glm::value_ptr(camera.camera));
    glUniformMatrix4fv(shaderColor_modelMatrix, 1, GL_FALSE, glm::value_ptr(glm::mat4()));

    glBindBuffer(GL_ARRAY_BUFFER, lowResVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lowResLineIBO);

    /* Draw a green wireframe sphere around the selected planet if there is one. */
    if (universe.isSelectedValid())
        drawPlanetWireframe(universe.getSelected());

    if (placing.step != PlacingInterface::NotPlacing && placing.step != PlacingInterface::Firing)
        drawPlanetWireframe(placing.planet);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    /* This color is used for trails, the velocity arrow when free placing, and the orbit circle when placing orbital. */
    glUniform4fv(shaderColor_color, 1, glm::value_ptr(glm::vec4(1.0f)));

    if (drawTrails) {
        /* There is no model matrix for drawing trails, they're in world space, just use identity. */
        glUniformMatrix4fv(shaderColor_modelMatrix, 1, GL_FALSE, glm::value_ptr(glm::mat4()));

        for (const auto& i : universe) {
            glVertexAttribPointer(vertex, 3, GL_FLOAT, GL_FALSE, 0, i.second.path.data());
            glDrawArrays(GL_LINE_STRIP, 0, GLsizei(i.second.path.size()));
        }
    }

   if (placing.step == PlacingInterface::FreeVelocity) {
        float length = glm::length(placing.planet.velocity) / universe.velocityfac;

        if (length > 0.0f) {
            glm::mat4 matrix = glm::translate(placing.planet.position);
            matrix = glm::scale(matrix, glm::vec3(placing.planet.radius()));
            matrix *= placing.rotation;
            glUniformMatrix4fv(shaderColor_modelMatrix, 1, GL_FALSE, glm::value_ptr(matrix));

            float verts[] = {  0.1f, 0.1f, 0.0f,
                               0.1f,-0.1f, 0.0f,
                              -0.1f,-0.1f, 0.0f,
                              -0.1f, 0.1f, 0.0f,

                               0.1f, 0.1f, length,
                               0.1f,-0.1f, length,
                              -0.1f,-0.1f, length,
                              -0.1f, 0.1f, length,

                               0.2f, 0.2f, length,
                               0.2f,-0.2f, length,
                              -0.2f,-0.2f, length,
                              -0.2f, 0.2f, length,

                               0.0f, 0.0f, length + 0.4f };

            static const GLubyte indexes[] = {  0,  1,  2,       2,  3,  0,

                                                1,  0,  5,       4,  5,  0,
                                                2,  1,  6,       5,  6,  1,
                                                3,  2,  7,       6,  7,  2,
                                                0,  3,  4,       7,  4,  3,

                                                5,  4,  9,       8,  9,  4,
                                                6,  5, 10,       9, 10,  5,
                                                7,  6, 11,      10, 11,  6,
                                                4,  7,  8,      11,  8,  7,

                                                9,  8, 12,
                                               10,  9, 12,
                                               11, 10, 12,
                                                8, 11, 12 };

            glVertexAttribPointer(vertex, 3, GL_FLOAT, GL_FALSE, 0, verts);
            glDrawElements(GL_TRIANGLES, sizeof(indexes), GL_UNSIGNED_BYTE, indexes);
        }
    }

   if (grid.draw) {
       grid.update(camera);

       glDepthMask(GL_FALSE);

       glVertexAttribPointer(vertex, 2, GL_FLOAT, GL_FALSE, 0, grid.points.data());

       glm::vec4 color = grid.color;
       color.a *= grid.alphafac;

       glUniformMatrix4fv(shaderColor_modelMatrix, 1, GL_FALSE, glm::value_ptr(glm::scale(glm::vec3(grid.scale))));
       glUniform4fv(shaderColor_color, 1, glm::value_ptr(color));

       glDrawArrays(GL_LINES, 0, GLsizei(grid.points.size()));

       glUniformMatrix4fv(shaderColor_modelMatrix, 1, GL_FALSE, glm::value_ptr(glm::scale(glm::vec3(grid.scale * 0.5f))));
       color.a = grid.color.a - color.a;
       glUniform4fv(shaderColor_color, 1, glm::value_ptr(color));

       glDrawArrays(GL_LINES, 0, GLsizei(grid.points.size()));

       glDepthMask(GL_TRUE);
   }

   glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, circleLineIBO);

   glVertexAttribPointer(vertex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

   if (placing.step == PlacingInterface::OrbitalPlane || placing.step == PlacingInterface::OrbitalPlanet &&
            universe.isSelectedValid() && placing.orbitalRadius > 0.0f) {
        glm::mat4 matrix = glm::translate(universe.getSelected().position);
        matrix = glm::scale(matrix, glm::vec3(placing.orbitalRadius));
        matrix *= placing.rotation;
        glUniformMatrix4fv(shaderColor_modelMatrix, 1, GL_FALSE, glm::value_ptr(matrix));

        glDrawElements(GL_LINES, circleLineCount, GL_UNSIGNED_INT, 0);
    }

    /* If there is a controller attatched, we aren't placing, and we aren't following anything, draw a little circle in the center of the screen. */
    if (controller != nullptr && placing.step == PlacingInterface::NotPlacing && camera.followingState == Camera::FollowNone) {
        glDisable(GL_DEPTH_TEST);

        glUniform4fv(shaderColor_color, 1, glm::value_ptr(glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)));

        glm::mat4 matrix = glm::translate(camera.position);
        matrix = glm::scale(matrix, glm::vec3(camera.distance * 4.0e-3f));
        glUniformMatrix4fv(shaderColor_modelMatrix, 1, GL_FALSE, glm::value_ptr(matrix));

        glDrawElements(GL_LINES, circleLineCount, GL_UNSIGNED_INT, 0);

        glEnable(GL_DEPTH_TEST);
    }
}

void PlanetsWindow::toggleFullscreen() {
    fullscreen = !fullscreen;

    SDL_SetWindowFullscreen(windowSDL, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}

void PlanetsWindow::doEvents() {
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            onClose();
            break;
        case SDL_KEYDOWN:
            doKeyPress(event.key.keysym);
            break;
        case SDL_WINDOWEVENT:
            switch(event.window.event) {
            case SDL_WINDOWEVENT_RESIZED:
                /* We don't want anyone resizing the window with a width or height of 0. */
                if (event.window.data1 < 1 || event.window.data2 < 1) {
                    SDL_SetWindowSize(windowSDL, std::max(event.window.data1, 1), std::max(event.window.data2, 1));
                    /* The above call will cause a new SDL_WINDOWEVENT_RESIZED event,
                     * so we break here and call onResized() from that event. */
                    break;
                }
                onResized(event.window.data1, event.window.data2);
                break;
            }
            break;
        case SDL_CONTROLLERDEVICEADDED:
            /* This will allow us to recieve events from the controller. */
            SDL_GameControllerOpen(event.cdevice.which);
            break;
        case SDL_CONTROLLERBUTTONUP:
            /* If we haven't picked a controller yet, use this one. */
            if (controller == nullptr || event.cbutton.button == SDL_CONTROLLER_BUTTON_GUIDE)
                controller = SDL_GameControllerOpen(event.cbutton.which);

            /* Ignore events from other controllers. */
            if (event.cbutton.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller)))
                doControllerButtonPress(event.cbutton.button);

            break;
        case SDL_MOUSEWHEEL:
            if (!placing.handleMouseWheel(event.wheel.y * 0.2f)) {
                camera.distance -= event.wheel.y * camera.distance * 0.1f;

                camera.bound();
            }
            break;
        case SDL_MOUSEBUTTONUP:
            if (event.button.button == SDL_BUTTON_LEFT) {
                glm::ivec2 pos(event.button.x, event.button.y);
                if(!placing.handleMouseClick(pos, camera))
                    camera.selectUnder(pos);
            }
            /* Always show cursor when mouse button is released. */
            SDL_SetRelativeMouseMode(SDL_FALSE);
            break;
        case SDL_MOUSEMOTION:
            /* yrel is inverted compared to the delta calculated in Qt. */
            glm::ivec2 delta(event.motion.xrel, -event.motion.yrel);

            bool holdCursor = false;

            if (!placing.handleMouseMove(glm::ivec2(event.motion.x, event.motion.y), delta, camera, holdCursor)) {
                if (event.motion.state & SDL_BUTTON_MMASK) {
                    camera.distance -= delta.y * camera.distance * 1.0e-2f;
                    camera.bound();
                } else if (event.motion.state & SDL_BUTTON_RMASK) {
                    camera.xrotation += delta.y * 0.01f;
                    camera.zrotation += delta.x * 0.01f;

                    camera.bound();

                    holdCursor = true;
                }
            }
            SDL_SetRelativeMouseMode(SDL_bool(holdCursor));
            break;
        }
    }
}

void PlanetsWindow::doKeyPress(const SDL_Keysym &key) {
    switch(key.sym) {
    case SDLK_ESCAPE:
        onClose();
        break;
    case SDLK_p:
        placing.beginInteractiveCreation();
        break;
    case SDLK_o:
        placing.beginOrbitalCreation();
        break;
    case SDLK_m:
        universe.generateRandomOrbital(1, universe.selected);
        break;
    case SDLK_r:
        universe.generateRandom(10, 1.0e3f, universe.velocityfac, 100.0f);
        break;
    case SDLK_c:
        universe.centerAll();
        break;
    case SDLK_DELETE:
        universe.deleteSelected();
        break;
    case SDLK_t:
        drawTrails = !drawTrails;
        break;
    case SDLK_g:
        grid.toggle();
        break;
    case SDLK_KP_PLUS:
        if(universe.simspeed <= 0.0f)
            universe.simspeed = 1.0f;
        else if(universe.simspeed < 64.0f)
            universe.simspeed *= 2.0f;
        break;
    case SDLK_KP_MINUS:
        if(universe.simspeed <= 1.0f)
            universe.simspeed = 0.0f;
        else
            universe.simspeed *= 0.5f;
        break;
    case SDLK_RETURN:
        if(key.mod & KMOD_ALT)
            toggleFullscreen();
        break;
    case SDLK_SPACE:
        if (universe.simspeed <= 0.0f)
            universe.simspeed = pauseSpeed;
        else {
            pauseSpeed = universe.simspeed;
            universe.simspeed = 0.0f;
        }
        break;
    case SDLK_LEFT:
        if (universe.stepsPerFrame > 1)
            --universe.stepsPerFrame;
        break;
    case SDLK_RIGHT:
        ++universe.stepsPerFrame;
        break;
    case SDLK_UP:
        if(universe.pathLength < 2000)
            universe.pathLength *= 2;
        else
            universe.pathLength = 4000;
        break;
    case SDLK_DOWN:
        if(universe.pathLength > 200)
            universe.pathLength /= 2;
        else
            universe.pathLength = 100;
        break;
    }
}

void PlanetsWindow::doControllerButtonPress(const Uint8 &button) {
    /* When simulating mouse events we use the center of the screen (in pixels). */
    glm::ivec2 centerScreen(windowWidth / 2, windowHeight / 2);

    switch(button) {
    case SDL_CONTROLLER_BUTTON_BACK:
        onClose();
        break;
    case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
        camera.reset();
        break;
    case SDL_CONTROLLER_BUTTON_LEFTSTICK:
        camera.position = glm::vec3();
        break;
    case SDL_CONTROLLER_BUTTON_A:
        /* TODO - there should probably be a seperate function for this... */
        if (!placing.handleMouseClick(centerScreen, camera))
            camera.selectUnder(centerScreen);
        break;
    case SDL_CONTROLLER_BUTTON_X:
        universe.deleteSelected();
        break;
    case SDL_CONTROLLER_BUTTON_Y:
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
        camera.followPrevious();
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
        camera.clearFollow();
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_UP:
        if (camera.followingState == Camera::WeightedAverage)
            camera.followPlainAverage();
        else
            camera.followWeightedAverage();
        break;
    }
}

void PlanetsWindow::doControllerAxisInput(int64_t delay) {
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

void PlanetsWindow::onClose() {
    const SDL_MessageBoxButtonData buttons[] = {
        { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, "No" },
        { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Yes" }
    };
    const SDL_MessageBoxData messageboxdata = {
        SDL_MESSAGEBOX_WARNING, windowSDL,
        "Exit Planets3D?", "The universe will not be saved.",
        SDL_arraysize(buttons), buttons, nullptr
    };

    int result;
    running = !universe.isEmpty() && SDL_ShowMessageBox(&messageboxdata, &result) == 0 && result == 0;
}

void PlanetsWindow::onResized(uint32_t width, uint32_t height) {
    /* Store the width and height for later use. */
    windowWidth = width;
    windowHeight = height;

    /* Resize the viewport and camera. */
    glViewport(0, 0, width, height);
    camera.resizeViewport(float(width), float(height));
}

void PlanetsWindow::drawPlanet(const Planet &planet) {
    glm::mat4 matrix = glm::translate(planet.position);
    matrix = glm::scale(matrix, glm::vec3(planet.radius()));
    glUniformMatrix4fv(shaderTexture_modelMatrix, 1, GL_FALSE, glm::value_ptr(matrix));

    glVertexAttribPointer(vertex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glVertexAttribPointer(uv,     2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(glm::vec3)));
    glDrawElements(GL_TRIANGLES, highResTriCount, GL_UNSIGNED_INT, 0);
}

void PlanetsWindow::drawPlanetWireframe(const Planet &planet, const uint32_t &color) {
    glUniform4fv(shaderColor_color, 1, glm::value_ptr(uintColorToVec4(color)));

    glm::mat4 matrix = glm::translate(planet.position);
    matrix = glm::scale(matrix, glm::vec3(planet.radius() * 1.05f));
    glUniformMatrix4fv(shaderColor_modelMatrix, 1, GL_FALSE, glm::value_ptr(matrix));

    glVertexAttribPointer(vertex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glDrawElements(GL_LINES, lowResLineCount, GL_UNSIGNED_INT, 0);
}

const GLuint PlanetsWindow::vertex = 0;
const GLuint PlanetsWindow::uv     = 1;
