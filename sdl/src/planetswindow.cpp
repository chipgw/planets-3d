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
#include <imgui/imgui.h>

#include "res.hpp"

PlanetsWindow::PlanetsWindow(int argc, char* argv[]) : placing(universe), camera(universe), gamepad(universe, camera, placing) {
    initSDL();
    initGL();
    initUI();

    gamepad.closeFunction = std::bind(&PlanetsWindow::onClose, this);

    /* Try loading from the command line. Ignore invalid files and break on the first successful file. */
    for (int i = 0; i < argc; ++i) {
        try {
            universe.load(argv[i]); break;
        } catch (...) {}
    }
}

PlanetsWindow::~PlanetsWindow() {
    /* Delete vertex & index buffers. */
    glDeleteBuffers(1, &highResVBO);
    glDeleteBuffers(1, &highResTriIBO);
    glDeleteBuffers(1, &lowResVBO);
    glDeleteBuffers(1, &lowResLineIBO);
    glDeleteBuffers(1, &circleVBO);
    glDeleteBuffers(1, &circleLineIBO);

    /* No more shaders. */
    glDeleteProgram(shaderTexture);
    glDeleteProgram(shaderColor);

    /* Die textures. */
    glDeleteTextures(1, &planetTexture_diff);
    glDeleteTextures(1, &planetTexture_nrm);

    /* Nice knowin ya OpenGL context. */
    SDL_GL_DeleteContext(contextSDL);

    /* Farewell SDL window. */
    SDL_DestroyWindow(windowSDL);

    /* Go away SDL, we don't need you any more. */
    SDL_Quit();
}

void PlanetsWindow::initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) == -1) {
        printf("ERROR: Unable to init SDL! \"%s\"", SDL_GetError());
        abort();
    }

    /* Use a 3.0 core profile. */
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS,  1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,  8);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    windowSDL = SDL_CreateWindow("Planets3D", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600,
                                 SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED);

    /* We're doomed... */
    if (!windowSDL) {
        printf("ERROR: Unable to create window!");
        abort();
    }

    contextSDL = SDL_GL_CreateContext(windowSDL);

    /* Giveus as many frames as possible. */
    SDL_GL_SetSwapInterval(0);

    /* We want to see the cursor. */
    SDL_ShowCursor(SDL_ENABLE);

    /* Events from gamepads are nice... */
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
    if (!GLEW_VERSION_3_0)
        puts("WARNING: OpenGL 3.0 support NOT detected, things probably won't work.");
#endif

    printf("GL Vendor: \"%s\", Renderer: \"%s\".\n", glGetString(GL_VENDOR), glGetString(GL_RENDERER));

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

    /* This attribute array is always on. */
    glEnableVertexAttribArray(vertex);

    glActiveTexture(GL_TEXTURE0);
    planetTexture_diff = loadTexture(SDL_RWFromConstMem(planet_diffuse_png, planet_diffuse_png_size));
    glActiveTexture(GL_TEXTURE1);
    planetTexture_nrm = loadTexture(SDL_RWFromConstMem(planet_nrm_png, planet_nrm_png_size));

    initBuffers();
}

unsigned int PlanetsWindow::loadTexture(SDL_RWops* io) {
    SDL_Surface* image = IMG_Load_RW(io, SDL_FALSE);

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

void PlanetsWindow::initShaders() {
    /* Compile the flat color shader included in res.h as const char*. */
    GLuint shaderColor_vsh = compileShader(color_vsh, GL_VERTEX_SHADER);
    GLuint shaderColor_fsh = compileShader(color_fsh, GL_FRAGMENT_SHADER);
    shaderColor = linkShaderProgram(shaderColor_vsh, shaderColor_fsh);

    /* Get the uniform locations from color shader. */
    glUseProgram(shaderColor);
    shaderColor_color           = glGetUniformLocation(shaderColor, "color");
    shaderColor_cameraMatrix    = glGetUniformLocation(shaderColor, "cameraMatrix");
    shaderColor_modelMatrix     = glGetUniformLocation(shaderColor, "modelMatrix");


    /* Compile the textured shader included in res.h as const char*. */
    GLuint shaderTexture_vsh = compileShader(texture_vsh, GL_VERTEX_SHADER);
    GLuint shaderTexture_fsh = compileShader(texture_fsh, GL_FRAGMENT_SHADER);

    shaderTexture = linkShaderProgram(shaderTexture_vsh, shaderTexture_fsh);

    /* Get the uniform locations from the texture shader. */
    glUseProgram(shaderTexture);
    shaderTexture_cameraMatrix  = glGetUniformLocation(shaderTexture, "cameraMatrix");
    shaderTexture_viewMatrix    = glGetUniformLocation(shaderTexture, "viewMatrix");
    shaderTexture_modelMatrix   = glGetUniformLocation(shaderTexture, "modelMatrix");
    shaderTexture_lightDir      = glGetUniformLocation(shaderTexture, "lightDir");

    glUniform1i(glGetUniformLocation(shaderTexture, "texture_diff"), 0);
    glUniform1i(glGetUniformLocation(shaderTexture, "texture_nrm"), 1);

    /* Compile the UI shader included in res.h as const char*. */
    GLuint shaderUI_vsh = compileShader(ui_vsh, GL_VERTEX_SHADER);
    GLuint shaderUI_fsh = compileShader(ui_fsh, GL_FRAGMENT_SHADER);
    shaderUI = linkShaderProgram(shaderUI_vsh, shaderUI_fsh);

    /* Get the uniform locations from UI shader. */
    glUseProgram(shaderUI);
    glUniform1i(glGetUniformLocation(shaderUI, "uiTexture"), 0);
    shaderUI_matrix         = glGetUniformLocation(shaderUI, "matrix");
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
    glBufferData(GL_ARRAY_BUFFER, circle.vertexCount * sizeof(glm::vec3), circle.verts, GL_STATIC_DRAW);

    glGenBuffers(1, &circleLineIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, circleLineIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, circle.lineCount * sizeof(uint32_t), circle.lines, GL_STATIC_DRAW);

    highResTriCount = highResSphere.triangleCount;
    lowResLineCount = lowResSphere.lineCount;
    circleLineCount = circle.lineCount;
}

static void interfaceRenderFunc(ImDrawData* drawData) {
    ImGuiIO& io = ImGui::GetIO();
    GLint viewport[4]; glGetIntegerv(GL_VIEWPORT, viewport);
    drawData->ScaleClipRects(io.DisplayFramebufferScale);

    glEnableVertexAttribArray(vertex);
    glEnableVertexAttribArray(uv);
    glEnableVertexAttribArray(normal);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    constexpr GLenum indexType = sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;

    for (int n = 0; n < drawData->CmdListsCount; ++n) {
        const ImDrawList* cmdList = drawData->CmdLists[n];
        intptr_t idxBufferOffset = 0;

        glVertexAttribPointer(vertex,   2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), &cmdList->VtxBuffer.front().pos);
        glVertexAttribPointer(uv,       2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), &cmdList->VtxBuffer.front().uv);
        glVertexAttribPointer(normal,   4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), &cmdList->VtxBuffer.front().col);

        for (const ImDrawCmd* pcmd = cmdList->CmdBuffer.begin(); pcmd != cmdList->CmdBuffer.end(); ++pcmd) {
            if (pcmd->UserCallback) {
                pcmd->UserCallback(cmdList, pcmd);
            } else {
                glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
                glScissor((int)pcmd->ClipRect.x, (int)(viewport[3] - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
                glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, indexType, &cmdList->IdxBuffer[idxBufferOffset]);
            }
            idxBufferOffset += pcmd->ElemCount;
        }
    }

    /* The defaults that everything else uses. */
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
}

void PlanetsWindow::initUI() {
    GLuint fontTexture;

    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);

    // Upload texture to graphics system
    glGenTextures(1, &fontTexture);
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, pixels);

    // Store our identifier
    io.Fonts->TexID = reinterpret_cast<void*>(static_cast<intptr_t>(fontTexture));

    // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
    io.KeyMap[ImGuiKey_Tab] = SDLK_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
    io.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = SDL_SCANCODE_PAGEUP;
    io.KeyMap[ImGuiKey_PageDown] = SDL_SCANCODE_PAGEDOWN;
    io.KeyMap[ImGuiKey_Home] = SDL_SCANCODE_HOME;
    io.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
    io.KeyMap[ImGuiKey_Delete] = SDLK_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = SDLK_BACKSPACE;
    io.KeyMap[ImGuiKey_Enter] = SDLK_RETURN;
    io.KeyMap[ImGuiKey_Escape] = SDLK_ESCAPE;
    io.KeyMap[ImGuiKey_A] = SDLK_a;
    io.KeyMap[ImGuiKey_C] = SDLK_c;
    io.KeyMap[ImGuiKey_V] = SDLK_v;
    io.KeyMap[ImGuiKey_X] = SDLK_x;
    io.KeyMap[ImGuiKey_Y] = SDLK_y;
    io.KeyMap[ImGuiKey_Z] = SDLK_z;

    io.RenderDrawListsFn = interfaceRenderFunc;

    io.SetClipboardTextFn = [](const char* text) { SDL_SetClipboardText(text); };
    io.GetClipboardTextFn = []() { return static_cast<const char*>(SDL_GetClipboardText()); };
}

void PlanetsWindow::run() {
    running = true;

    typedef std::chrono::high_resolution_clock clock;
    using std::chrono::duration_cast;
    using std::chrono::microseconds;

    /* To track the total running time of the program. */
    clock::time_point start_time = clock::now();
    /* To track the time spent per frame. */
    clock::time_point last_time = start_time;

    while (running) {
        /* Figure out how long the last frame took to render & display, in microseconds. */
        clock::time_point current = clock::now();
        int delay = duration_cast<microseconds>(current - last_time).count();
        last_time = current;

        if (delay != 0)
            /* Put a bunch of information into the title. */
            SDL_SetWindowTitle(windowSDL, ("Planets3D  [" + std::to_string(1000000 / delay) + "fps, " + std::to_string(delay / 1000) + "ms " +
                                           std::to_string(universe.size()) + " planet(s), " + std::to_string(universe.simspeed) + "x speed, " +
                                           std::to_string(universe.stepsPerFrame) + " step(s), " +
                                           std::to_string(universe.pathLength) + " path length]").c_str());

        /* Don't do delays larger than a second. */
        delay = std::min(delay, 1000000);

        gamepad.doControllerAxisInput(delay);
        doEvents();

        /* Don't advance if we're placing. */
        if (placing.step == PlacingInterface::NotPlacing || placing.step == PlacingInterface::Firing)
            universe.advance(float(delay));

        paint();
        paintUI(delay * 1.0e-6f);


        SDL_GL_SwapWindow(windowSDL);

        ++totalFrames;

        /* So anything that was written to console gets written. */
        fflush(stdout);
    }

    typedef std::chrono::duration<double> dsec;
    typedef std::chrono::duration<double, std::milli> dms;

    /* Output stats to the console. */
    clock::time_point current = clock::now();
    printf("Total Time: %f seconds.\n", (duration_cast<dsec>(current - start_time).count()));
    printf("Total Frames: %ju.\n", totalFrames);
    printf("Average Draw Time: %fms.\n", duration_cast<dms>(current - start_time).count() / double(totalFrames));
    printf("Average Framerate: %f fps.\n", 1.0 / (duration_cast<dsec>(current - start_time).count() / double(totalFrames)));
}

void PlanetsWindow::paint() {
    /* Make sure we're using the right GL context and clear the window. */
    SDL_GL_MakeCurrent(windowSDL, contextSDL);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    camera.setup();

    /* We only use the texture shader, normals, tangents, and uv's for drawing the shaded planets. */
    glUseProgram(shaderTexture);
    glEnableVertexAttribArray(normal);
    glEnableVertexAttribArray(tangent);
    glEnableVertexAttribArray(uv);

    /* Update the light direction in view space. */
    glm::vec3 light = glm::vec3(0.57735f);
    light = glm::vec3(camera.view * glm::vec4(light, 0.0f));
    glUniform3fv(shaderTexture_lightDir, 1, glm::value_ptr(light));

    /* Upload the updated camera matrix. */
    glUniformMatrix4fv(shaderTexture_cameraMatrix, 1, GL_FALSE, glm::value_ptr(camera.camera));
    glUniformMatrix4fv(shaderTexture_viewMatrix, 1, GL_FALSE, glm::value_ptr(camera.view));

    glBindBuffer(GL_ARRAY_BUFFER, highResVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, highResTriIBO);

    glVertexAttribPointer(vertex,   3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glVertexAttribPointer(normal,   3, GL_FLOAT, GL_FALSE, sizeof(Vertex), &(((Vertex*)(0))->normal));
    glVertexAttribPointer(tangent,  3, GL_FLOAT, GL_FALSE, sizeof(Vertex), &(((Vertex*)(0))->tangent));
    glVertexAttribPointer(uv,       2, GL_FLOAT, GL_FALSE, sizeof(Vertex), &(((Vertex*)(0))->uv));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, planetTexture_diff);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, planetTexture_nrm);

    for (const auto& i : universe) {
        glm::mat4 matrix = glm::translate(i.position);
        matrix = glm::scale(matrix, glm::vec3(i.radius()));
        glUniformMatrix4fv(shaderTexture_modelMatrix, 1, GL_FALSE, glm::value_ptr(matrix));
        glDrawElements(GL_TRIANGLES, highResTriCount, GL_UNSIGNED_INT, 0);
    }

    /* Now the texture shader and uv's don't get used until next frame. */
    glDisableVertexAttribArray(normal);
    glDisableVertexAttribArray(tangent);
    glDisableVertexAttribArray(uv);
    glUseProgram(shaderColor);

    glUniformMatrix4fv(shaderColor_cameraMatrix, 1, GL_FALSE, glm::value_ptr(camera.camera));
    glUniformMatrix4fv(shaderColor_modelMatrix, 1, GL_FALSE, glm::value_ptr(glm::mat4()));

    /* Bind the low resolution wireframe sphere. */
    glBindBuffer(GL_ARRAY_BUFFER, lowResVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lowResLineIBO);
    glVertexAttribPointer(vertex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

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
            glVertexAttribPointer(vertex, 3, GL_FLOAT, GL_FALSE, 0, i.path.data());
            glDrawArrays(GL_LINE_STRIP, 0, GLsizei(i.path.size()));
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
        /* Update the grid's scale and alphafac based on the camera. */
        grid.update(camera);

        glDepthMask(GL_FALSE);

        /* TODO - Store in VBO. */
        glVertexAttribPointer(vertex, 2, GL_FLOAT, GL_FALSE, 0, grid.points.data());

        glm::vec4 color = grid.color;
        /* The alphafac value is for the larger of the two grids. */
        color.a *= grid.alphafac;

        glUniformMatrix4fv(shaderColor_modelMatrix, 1, GL_FALSE, glm::value_ptr(glm::scale(glm::vec3(grid.scale))));
        glUniform4fv(shaderColor_color, 1, glm::value_ptr(color));

        glDrawArrays(GL_LINES, 0, GLsizei(grid.points.size()));

        glUniformMatrix4fv(shaderColor_modelMatrix, 1, GL_FALSE, glm::value_ptr(glm::scale(glm::vec3(grid.scale * 0.5f))));
        /* The smaller grid disappears as the big one appears. */
        color.a = grid.color.a - color.a;
        glUniform4fv(shaderColor_color, 1, glm::value_ptr(color));

        glDrawArrays(GL_LINES, 0, GLsizei(grid.points.size()));

        /* Draw to the depth buffer again. */
        glDepthMask(GL_TRUE);
    }

    if (drawPlanarCircles) {
        /* First draw the lines from the circle center to the planet location. */
        glUniform4fv(shaderColor_color, 1, glm::value_ptr(glm::vec4(0.8f)));
        glUniformMatrix4fv(shaderColor_modelMatrix, 1, GL_FALSE, glm::value_ptr(glm::mat4()));

        for (const auto& i : universe) {
            float verts[] = {
                i.position.x, i.position.y, 0,
                i.position.x, i.position.y, i.position.z,
            };
            glVertexAttribPointer(vertex, 3, GL_FLOAT, GL_FALSE, 0, verts);
            glDrawArrays(GL_LINES, 0, 2);
        }
    }

    /* Binding the circle is used for planar circles, the orbital circle, and the controller center. */
    glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, circleLineIBO);

    glVertexAttribPointer(vertex, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);

    if (drawPlanarCircles) {
        /* Then we draw a circle at the XY position of every planet. */
        for (const auto& i : universe) {
            glm::vec3 pos = i.position;
            pos.z = 0;

            glm::mat4 matrix = glm::translate(pos);
            /* Make the circle start at the planet's radius and increase it slightly the further out the camera is. */
            matrix = glm::scale(matrix, glm::vec3(i.radius() + camera.distance * 0.02f));

            glUniformMatrix4fv(shaderColor_modelMatrix, 1, GL_FALSE, glm::value_ptr(matrix));
            glDrawElements(GL_LINES, circleLineCount, GL_UNSIGNED_INT, nullptr);
        }
    }

    if ((placing.step == PlacingInterface::OrbitalPlane || placing.step == PlacingInterface::OrbitalPlanet)
            && universe.isSelectedValid() && placing.orbitalRadius > 0.0f) {
        glUniform4fv(shaderColor_color, 1, glm::value_ptr(glm::vec4(1.0f)));

        glm::mat4 newRadiusMatrix = placing.getOrbitalCircleMat();
        glm::mat4 oldRadiusMatrix = placing.getOrbitedCircleMat();

        /* Draw both circles. */
        glUniformMatrix4fv(shaderColor_modelMatrix, 1, GL_FALSE, glm::value_ptr(newRadiusMatrix));
        glDrawElements(GL_LINES, circleLineCount, GL_UNSIGNED_INT, nullptr);

        glUniformMatrix4fv(shaderColor_modelMatrix, 1, GL_FALSE, glm::value_ptr(oldRadiusMatrix));
        glDrawElements(GL_LINES, circleLineCount, GL_UNSIGNED_INT, nullptr);
    }

    /* If there is a controller attached, we aren't placing, and we aren't following anything, draw a little circle in the center of the screen. */
    if (gamepad.isAttached() && placing.step == PlacingInterface::NotPlacing && camera.followingState == Camera::FollowNone) {
        glDisable(GL_DEPTH_TEST);

        glUniform4fv(shaderColor_color, 1, glm::value_ptr(glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)));

        glm::mat4 matrix = glm::translate(camera.position);
        matrix = glm::scale(matrix, glm::vec3(camera.distance * 4.0e-3f));
        glUniformMatrix4fv(shaderColor_modelMatrix, 1, GL_FALSE, glm::value_ptr(matrix));

        glDrawElements(GL_LINES, circleLineCount, GL_UNSIGNED_INT, 0);

        glEnable(GL_DEPTH_TEST);
    }
}

void PlanetsWindow::paintUI(const float delay) {
    ImGuiIO& io = ImGui::GetIO();

    io.DeltaTime = delay;

    // Setup inputs
    // (we already got mouse wheel, keyboard keys & characters from SDL_PollEvent())
    int mx, my;
    Uint32 mouseMask = SDL_GetMouseState(&mx, &my);
    if (SDL_GetWindowFlags(windowSDL) & SDL_WINDOW_MOUSE_FOCUS)
        io.MousePos = ImVec2((float)mx, (float)my);   // Mouse position, in pixels (set to -1,-1 if no mouse / on another screen, etc.)
    else
        io.MousePos = ImVec2(-1, -1);

    // If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
    io.MouseDown[0] = /*g_MousePressed[0] ||*/ (mouseMask & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
    io.MouseDown[1] = /*g_MousePressed[1] ||*/ (mouseMask & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
    io.MouseDown[2] = /*g_MousePressed[2] ||*/ (mouseMask & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
//        g_MousePressed[0] = g_MousePressed[1] = g_MousePressed[2] = false;

//        io.MouseWheel = g_MouseWheel;
//        g_MouseWheel = 0.0f;

    /* Hide OS cursor if imgui has one. */
    SDL_ShowCursor(io.MouseDrawCursor ? SDL_FALSE : SDL_TRUE);

    /* Begin UI code. */
    ImGui::NewFrame();

    static bool show_test_window = false;
    if (ImGui::Button("Foo"))
        show_test_window = !show_test_window;

    if (show_test_window) {
        ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
        ImGui::ShowTestWindow(&show_test_window);
    }

    /* TODO - Actual UI code. */

    /* End UI code. */

    /* Begin rendering code. */
    int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
    int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
    if (fb_width == 0 || fb_height == 0)
        return;

    // Backup GL state
    GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);

    // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glActiveTexture(GL_TEXTURE0);

    // Setup orthographic projection matrix
    glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
    const glm::mat4 projection( 2.0f/io.DisplaySize.x, 0.0f,                   0.0f, 0.0f,
                                0.0f,                  2.0f/-io.DisplaySize.y, 0.0f, 0.0f,
                                0.0f,                  0.0f,                  -1.0f, 0.0f,
                               -1.0f,                  1.0f,                   0.0f, 1.0f);

    glUseProgram(shaderUI);
    glUniformMatrix4fv(shaderUI_matrix, 1, GL_FALSE, glm::value_ptr(projection));

    ImGui::Render();
    /* End rendering code. */

    glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
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
        case SDL_CONTROLLERBUTTONUP:
            gamepad.handleEvent(event);
            break;
        case SDL_MOUSEWHEEL:
            if (!placing.handleMouseWheel(event.wheel.y * 0.2f)) {
                camera.distance -= event.wheel.y * camera.distance * 0.1f;

                camera.bound();
            }
            break;
        case SDL_MOUSEBUTTONUP:
            if (event.button.button == SDL_BUTTON_LEFT) {
                if (event.button.clicks == 2 && placing.step == PlacingInterface::NotPlacing) {
                    if (universe.isSelectedValid())
                        camera.followSelection();
                    else
                        camera.clearFollow();
                } else {
                    glm::ivec2 pos(event.button.x, event.button.y);
                    if (!placing.handleMouseClick(pos, camera))
                        camera.selectUnder(pos);
                }
            } else if ((event.button.button == SDL_BUTTON_MIDDLE || event.button.button == SDL_BUTTON_RIGHT) && event.button.clicks == 2) {
                camera.reset();
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

void PlanetsWindow::doKeyPress(const SDL_Keysym& key) {
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
    case SDLK_n:
        if (key.mod & KMOD_CTRL)
            newUniverse();
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
    case SDLK_y:
        drawPlanarCircles = !drawPlanarCircles;
        break;
    case SDLK_g:
        grid.toggle();
        break;
    case SDLK_KP_PLUS:
        if (universe.simspeed <= 0.0f)
            universe.simspeed = 0.25f;
        else if (universe.simspeed < 64.0f)
            universe.simspeed *= 2.0f;
        break;
    case SDLK_KP_MINUS:
        if (universe.simspeed <= 0.25f)
            universe.simspeed = 0.0f;
        else
            universe.simspeed *= 0.5f;
        break;
    case SDLK_RETURN:
        if (key.mod & KMOD_ALT)
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
        if (universe.pathLength < 2000)
            universe.pathLength *= 2;
        else
            universe.pathLength = 4000;
        break;
    case SDLK_DOWN:
        if (universe.pathLength > 200)
            universe.pathLength /= 2;
        else
            universe.pathLength = 100;
        break;
    }
}

const SDL_MessageBoxButtonData buttonsNoYes[] = {
    { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, "No" },
    { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Yes" }
};

void PlanetsWindow::onClose() {
    const SDL_MessageBoxData messageboxdata = {
        SDL_MESSAGEBOX_WARNING, windowSDL,
        "Exit Planets3D?", "The universe will not be saved.",
        SDL_arraysize(buttonsNoYes), buttonsNoYes, nullptr
    };

    int result;
    running = !universe.isEmpty() && SDL_ShowMessageBox(&messageboxdata, &result) == 0 && result == 0;
}

void PlanetsWindow::newUniverse() {
    const SDL_MessageBoxData messageboxdata = {
        SDL_MESSAGEBOX_WARNING, windowSDL,
        "Destroy Universe?", "The universe will not be saved.",
        SDL_arraysize(buttonsNoYes), buttonsNoYes, nullptr
    };

    int result;
    if (!universe.isEmpty() && SDL_ShowMessageBox(&messageboxdata, &result) == 0 && result == 1)
        universe.deleteAll();
}

void PlanetsWindow::onResized(uint32_t width, uint32_t height) {
    /* Store the width and height for later use. */
    windowSize = glm::ivec2(width, height);

    /* Resize the viewport and camera. */
    glViewport(0, 0, width, height);
    camera.resizeViewport(float(width), float(height));

    ImGuiIO& io = ImGui::GetIO();
    int display_w, display_h;
    SDL_GL_GetDrawableSize(windowSDL, &display_w, &display_h);
    io.DisplaySize = ImVec2(static_cast<float>(width), static_cast<float>(height));
    io.DisplayFramebufferScale = ImVec2(width > 0 ? (static_cast<float>(display_w) / static_cast<float>(width)) : 0,
                                        height > 0 ? (static_cast<float>(display_h) / static_cast<float>(height)) : 0);
}

void PlanetsWindow::drawPlanetWireframe(const Planet& planet, const uint32_t& color) {
    glUniform4fv(shaderColor_color, 1, glm::value_ptr(uintColorToVec4(color)));

    glm::mat4 matrix = glm::translate(planet.position);
    matrix = glm::scale(matrix, glm::vec3(planet.radius() * 1.05f));
    glUniformMatrix4fv(shaderColor_modelMatrix, 1, GL_FALSE, glm::value_ptr(matrix));

    glDrawElements(GL_LINES, lowResLineCount, GL_UNSIGNED_INT, 0);
}
