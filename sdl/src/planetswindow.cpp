#include "planetswindow.h"
#include "shaders.h"
#include "spheregenerator.h"
#include "version.h"
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>
#include <SDL_image.h>
#include <imgui/imgui.h>
#include <sstream>

#include "res.hpp"

#define NUM_PLANET_TEXTURES 7

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
    glDeleteBuffers(1, &staticDataVBO);
    glDeleteBuffers(1, &staticDataIBO);
    glDeleteBuffers(1, &gridVBO);

    /* No more shaders. */
    glDeleteProgram(shaderTexture);
    glDeleteProgram(shaderColor);
    glDeleteProgram(shaderUI);

    /* Die textures. */
    glDeleteTextures(1, &planetTextures_diff);
    glDeleteTextures(1, &planetTextures_nrm);
    glDeleteTextures(1, &planetTextures_height);

    GLuint fontTexture = static_cast<GLuint>(reinterpret_cast<intptr_t>(ImGui::GetIO().Fonts->TexID));
    glDeleteTextures(1, &fontTexture);

    ImGui::DestroyContext();

    for (SDL_Cursor* c : cursors)
        SDL_FreeCursor(c);

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
        puts("ERROR: Unable to create window!");
        abort();
    }

    contextSDL = SDL_GL_CreateContext(windowSDL);

    /* Giveus as many frames as possible. */
    SDL_GL_SetSwapInterval(0);

    /* We want to see the cursor. */
    SDL_ShowCursor(SDL_ENABLE);

    /* Events from gamepads are nice... */
    SDL_GameControllerEventState(SDL_ENABLE);

    /* The cursors desired by ImGui. */
    cursors[0] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    cursors[1] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
    cursors[2] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
    cursors[3] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
    cursors[4] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
    cursors[5] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW);
    cursors[6] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
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

    std::stringstream info;
    info << "Version:      " << glGetString(GL_VERSION) << std::endl
         << "Vendor:       " << glGetString(GL_VENDOR) << std::endl
         << "Renderer:     " << glGetString(GL_RENDERER) << std::endl
         << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    glInfo = info.str();

    printf("OpenGL Info:\n%s", glInfo.data());

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

    planetTextures_diff = loadTextures({"planet_diffuse.png",  "planet_diffuse_01.png", "planet_diffuse_02.png", "planet_diffuse_03.png",
                                        "planet_diffuse_04.png", "planet_diffuse_05.png", "planet_diffuse_06.png"});
    planetTextures_nrm = loadTextures({"planet_nrm.png", "planet_nrm_01.png", "planet_nrm_02.png", "planet_nrm_03.png",
                                       "planet_nrm_04.png", "planet_nrm_05.png", "planet_nrm_06.png"});
    planetTextures_height = loadTextures({"planet_height.png", "planet_height_01.png", "planet_height_02.png", "planet_height_03.png",
                                          "planet_height_04.png", "planet_height_05.png", "planet_height_06.png"});

    initBuffers();
}

unsigned int PlanetsWindow::loadTextures(std::vector<std::string> files) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
    /* TODO - Unhardcode texture size. */
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 4, GL_RGBA8, 2048, 2048, GLsizei(files.size()));

    for (int i = 0; i < files.size(); ++i) {
        SDL_Surface* image = IMG_Load(("textures/" + files[i]).c_str());

        if (image == nullptr) {
            std::string err(IMG_GetError());

            /* Qt Creator's output panel doesn't seem to like the '\r' character for some reason... */
            auto ret = err.find('\r');
            if (ret > -1)
                err.erase(ret);

            printf("Failed to load texture! Error: %s\n", err.c_str());

            continue;
        }

        /* We need the format to be R8G8B8A8 for upload to GPU. */
        SDL_Surface* converted = SDL_ConvertSurface(image, SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888), 0);
        SDL_FreeSurface(image);

        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, converted->w, converted->h, 1, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, converted->pixels);

        SDL_FreeSurface(converted);
    }

    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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
    shaderTexture_material      = glGetUniformLocation(shaderTexture, "materialID");

    glUniform1i(glGetUniformLocation(shaderTexture, "texture_diff"), 0);
    glUniform1i(glGetUniformLocation(shaderTexture, "texture_nrm"), 1);
    glUniform1i(glGetUniformLocation(shaderTexture, "texture_height"), 2);

    /* Compile the UI shader included in res.h as const char*. */
    GLuint shaderUI_vsh = compileShader(ui_vsh, GL_VERTEX_SHADER);
    GLuint shaderUI_fsh = compileShader(ui_fsh, GL_FRAGMENT_SHADER);
    shaderUI = linkShaderProgram(shaderUI_vsh, shaderUI_fsh);

    /* Get the uniform locations from UI shader. */
    glUseProgram(shaderUI);
    glUniform1i(glGetUniformLocation(shaderUI, "uiTexture"), 0);
    shaderUI_matrix = glGetUniformLocation(shaderUI, "matrix");
}

void setupVertexArray(GLuint& vao, GLuint ibo, GLint stride, std::initializer_list<std::tuple<VertexAttrib, GLint, intptr_t>> attribs) {
    glGenVertexArrays(1, &vao); glBindVertexArray(vao);

    for (const auto& i : attribs) {
        glEnableVertexAttribArray(std::get<0>(i));
        glVertexAttribPointer(std::get<0>(i), std::get<1>(i), GL_FLOAT, GL_FALSE, stride, (GLvoid*)std::get<2>(i));
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
}

void PlanetsWindow::initBuffers() {
    const float verts[] = {  0.1f, 0.1f, 0.0f,
                             0.1f,-0.1f, 0.0f,
                            -0.1f,-0.1f, 0.0f,
                            -0.1f, 0.1f, 0.0f,

                             0.1f, 0.1f, 1.0f,
                             0.1f,-0.1f, 1.0f,
                            -0.1f,-0.1f, 1.0f,
                            -0.1f, 0.1f, 1.0f,

                             0.2f, 0.2f, 1.0f,
                             0.2f,-0.2f, 1.0f,
                            -0.2f,-0.2f, 1.0f,
                            -0.2f, 0.2f, 1.0f,

                             0.0f, 0.0f, 1.3f };

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

    IcoSphere highResSphere(5);
    IcoSphere lowResSphere(2);
    Circle circle(64);

    /* We actually just ignore the low res vertex data, because it's identical to the same range on the high res sphere. */
    intptr_t highResVertBufSize = highResSphere.vertexCount * sizeof(Vertex);
    intptr_t circleVertBufSize = circle.vertexCount * sizeof(glm::vec3);
    intptr_t arrowVertBufSize = sizeof(verts);

    intptr_t highResIndexBufSize = highResSphere.triangleCount * sizeof(uint32_t);
    intptr_t lowResIndexBufSize = lowResSphere.lineCount * sizeof(uint32_t);
    intptr_t circleIndexBufSize = circle.lineCount * sizeof(uint32_t);
    intptr_t arrowIndexBufSize = sizeof(indexes);

    glGenBuffers(1, &staticDataVBO); glBindBuffer(GL_ARRAY_BUFFER, staticDataVBO);
    glGenBuffers(1, &staticDataIBO); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, staticDataIBO);

    glBufferData(GL_ARRAY_BUFFER, highResVertBufSize + circleVertBufSize + arrowVertBufSize, 0, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0,                                         highResVertBufSize, highResSphere.verts);
    glBufferSubData(GL_ARRAY_BUFFER, highResVertBufSize,                        circleVertBufSize,  circle.verts);
    glBufferSubData(GL_ARRAY_BUFFER, highResVertBufSize + circleVertBufSize,    arrowVertBufSize,   verts);

    highResTriStart = 0;
    lowResLineStart = highResIndexBufSize;
    circleLineStart = highResIndexBufSize + lowResIndexBufSize;
    arrowTriStart = circleLineStart + circleIndexBufSize;

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, highResIndexBufSize + lowResIndexBufSize + circleIndexBufSize + arrowIndexBufSize, 0, GL_STATIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, highResTriStart, highResIndexBufSize,  highResSphere.triangles);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, lowResLineStart, lowResIndexBufSize,   lowResSphere.lines);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, circleLineStart, circleIndexBufSize,   circle.lines);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, arrowTriStart,   arrowIndexBufSize,    indexes);

    highResTriCount = highResSphere.triangleCount;
    lowResLineCount = lowResSphere.lineCount;
    circleLineCount = circle.lineCount;
    arrowTriCount = arrowIndexBufSize;

    /* Set up the vertex array object for the high res sphere. */
    setupVertexArray(highResSphereVAO, staticDataIBO, sizeof(Vertex), { {vertex, 3, 0}, {tangent, 3, offsetof(Vertex, tangent)}, {uv, 2, offsetof(Vertex, uv)} });
    setupVertexArray(lowResSphereVAO, staticDataIBO, sizeof(Vertex), {{vertex, 3, 0}});
    setupVertexArray(circleVAO, staticDataIBO, 0, {{vertex, 3, highResVertBufSize}});
    setupVertexArray(arrowVAO, staticDataIBO, 0, {{vertex, 3, highResVertBufSize + circleVertBufSize}});

    glBindVertexArray(0);

    /* Set up an ampty buffer for the grid. */
    glGenBuffers(1, &gridVBO); glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
    glBufferData(GL_ARRAY_BUFFER, 0, 0, GL_DYNAMIC_DRAW);
}

void PlanetsWindow::initUI() {
    ImGui::CreateContext();

    GLuint fontTexture;

    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);

    /* Upload the font texture to OpenGL. */
    glGenTextures(1, &fontTexture);
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, pixels);

    /* Store the font texture handle. */
    io.Fonts->TexID = reinterpret_cast<void*>(static_cast<intptr_t>(fontTexture));

    /* Texture is uploaded, we don't need the font pixel data any more. */
    io.Fonts->ClearTexData();

    /* Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array. */
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

    io.SetClipboardTextFn = [](void*, const char* text) { SDL_SetClipboardText(text); };
    io.GetClipboardTextFn = [](void*) { return static_cast<const char*>(SDL_GetClipboardText()); };

    ImGuiStyle& style = ImGui::GetStyle();

    /* Set the window background to stand out from the universe background. */
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.2f, 0.2f, 0.3f, 0.50f);

    /* Reduce the window corner rounding. */
    style.WindowRounding = 4.0f;
}

#ifdef PLANETS3D_WITH_NFD
#include <nfd.h>

void PlanetsWindow::openFile() {
    nfdchar_t* outPath = NULL;
    nfdresult_t result = NFD_OpenDialog("xml", NULL, &outPath);

    if (result == NFD_OKAY) {
        universe.load(outPath);
        free(outPath);
    } else if (result == NFD_ERROR)
        printf("Error: %s\n", NFD_GetError());
}

void PlanetsWindow::appendFile() {
    nfdchar_t* outPath = NULL;
    nfdresult_t result = NFD_OpenDialog("xml", NULL, &outPath);

    if (result == NFD_OKAY) {
        universe.load(outPath, false);
        free(outPath);
    } else if (result == NFD_ERROR)
        printf("Error: %s\n", NFD_GetError());
}

void PlanetsWindow::saveFile() {
    nfdchar_t* outPath = NULL;
    nfdresult_t result = NFD_SaveDialog("xml", NULL, &outPath);

    if (result == NFD_OKAY) {
        universe.save(outPath);
        free(outPath);
    } else if (result == NFD_ERROR)
        printf("Error: %s\n", NFD_GetError());
}
#endif /* PLANETS3D_WITH_NFD */

void PlanetsWindow::run() {
    /* Remains true from here until application closes. */
    running = true;

    /* To track the time spent per frame. */
    Uint64 last_time = SDL_GetPerformanceCounter();

    while (running) {
        /* Figure out how long the last frame took to render & display, in microseconds. */
        Uint64 current = SDL_GetPerformanceCounter();
        int delay = static_cast<int>((current - last_time)*1000000 / SDL_GetPerformanceFrequency());
        last_time = current;

        /* Store in milliseconds. */
        frameTimes[frameTimeOffset++] = delay * 1.0e-3f;
        if (frameTimeOffset >= frameTimes.size())
            frameTimeOffset = 0;

        if (delay != 0)
            /* Put a bunch of information into the title. */
            SDL_SetWindowTitle(windowSDL, ("Planets3D  [" + std::to_string(1000000 / delay) + "fps, " +
                                           std::to_string(universe.size()) + " planet(s)]").c_str());

        /* Don't do delays larger than a second. */
        delay = std::min(delay, 1000000);

        gamepad.doControllerAxisInput(delay);
        doEvents();

        /* Don't advance if we're placing. */
        if (placing.step == PlacingInterface::NotPlacing || placing.step == PlacingInterface::Firing)
            universe.advance(float(delay));

        paint();
        /* UI time is measured in seconds. */
        paintUI(delay * 1.0e-6f);

        SDL_GL_SwapWindow(windowSDL);

        ++totalFrames;

        /* So anything that was written to console gets written. */
        fflush(stdout);
    }

    /* Output stats to the console. */
    printf("Total Time: %f seconds.\n", SDL_GetTicks() * 1.0e-3);
    printf("Total Frames: %ju.\n", totalFrames);
    printf("Average Draw Time: %fms.\n", SDL_GetTicks() / double(totalFrames));
    printf("Average Framerate: %f fps.\n", 1.0 / (SDL_GetTicks() * 1.0e-3 / double(totalFrames)));
}

void PlanetsWindow::paint() {
    /* Make sure we're using the right GL context and clear the window. */
    SDL_GL_MakeCurrent(windowSDL, contextSDL);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    camera.setup();

    /* We only use the texture shader, normals, tangents, and uvs for drawing the shaded planets. */
    glUseProgram(shaderTexture);

    /* This is just vec3(1.0) normalized. */
    glm::vec3 light = glm::vec3(0.57735f);
    /* Put the light direction into view space. */
    light = glm::vec3(camera.view * glm::vec4(light, 0.0f));
    glUniform3fv(shaderTexture_lightDir, 1, glm::value_ptr(light));

    /* Upload the updated camera matrix. */
    glUniformMatrix4fv(shaderTexture_cameraMatrix, 1, GL_FALSE, glm::value_ptr(camera.camera));
    glUniformMatrix4fv(shaderTexture_viewMatrix, 1, GL_FALSE, glm::value_ptr(camera.view));

    glBindVertexArray(highResSphereVAO);

    std::uniform_int_distribution<int> material(0, NUM_PLANET_TEXTURES-1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, planetTextures_diff);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D_ARRAY, planetTextures_nrm);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D_ARRAY, planetTextures_height);

    for (Planet& planet : universe) {
        /* If the material is invalid, generate a valid one. */
        if (planet.materialID > NUM_PLANET_TEXTURES)
            planet.materialID = material(universe.generator);

        glUniform1i(shaderTexture_material, planet.materialID);

        /* Create a matrix translated by the position and scaled by the radius. */
        glm::mat4 matrix = glm::translate(planet.position);
        matrix = glm::scale(matrix, glm::vec3(planet.radius() * drawScale));
        glUniformMatrix4fv(shaderTexture_modelMatrix, 1, GL_FALSE, glm::value_ptr(matrix));

        /* Render all the triangles... */
        glDrawElements(GL_TRIANGLES, highResTriCount, GL_UNSIGNED_INT, (GLvoid*)highResTriStart);
    }

    /* Everything else (other than UI) uses the flat color shader. */
    glUseProgram(shaderColor);

    /* The color shader needs the camera updated too. */
    glUniformMatrix4fv(shaderColor_cameraMatrix, 1, GL_FALSE, glm::value_ptr(camera.camera));

    /* Bind the low resolution wireframe sphere. */
    glBindVertexArray(lowResSphereVAO);

    /* Draw a green wireframe sphere around the selected planet if there is one. */
    if (universe.isSelectedValid())
        drawPlanetWireframe(universe.getSelected());

    /* For any valid placing states other than firing, draw the templateplanet. */
    if (placing.step != PlacingInterface::NotPlacing && placing.step != PlacingInterface::Firing)
        drawPlanetWireframe(placing.planet);

    /* This color is used for trails, the velocity arrow when free placing, and the orbit circle when placing orbital. */
    glUniform4fv(shaderColor_color, 1, glm::value_ptr(glm::vec4(1.0f)));

    if (placing.step == PlacingInterface::FreeVelocity && !glm::all(glm::equal(placing.planet.velocity, glm::vec3()))) {
        glBindVertexArray(arrowVAO);

        /* How long does the velocity arrow need to be? */
        float length = glm::length(placing.planet.velocity) / universe.velocityFactor;

        glm::mat4 matrix = glm::translate(placing.planet.position);
        matrix *= placing.rotation;
        matrix = glm::scale(matrix, glm::vec3(placing.planet.radius()) * glm::vec3(1.0f, 1.0f, length));
        glUniformMatrix4fv(shaderColor_modelMatrix, 1, GL_FALSE, glm::value_ptr(matrix));

        glDrawElements(GL_TRIANGLES, arrowTriCount, GL_UNSIGNED_BYTE, (GLvoid*)arrowTriStart);
    }

    /* Back to no VAO. */
    glBindVertexArray(0);

    if (drawTrails) {
        /* There is no model matrix for drawing trails, they're in world space, just use identity. */
        glUniformMatrix4fv(shaderColor_modelMatrix, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));

        for (const auto& i : universe) {
            glVertexAttribPointer(vertex, 3, GL_FLOAT, GL_FALSE, 0, i.path.data());
            glDrawArrays(GL_LINE_STRIP, 0, GLsizei(i.path.size()));
        }
    }

    if (grid.draw) {
        glDepthMask(GL_FALSE);

        /* Bind the grid buffer for drawing and (optionally) updating. */
        glBindBuffer(GL_ARRAY_BUFFER, gridVBO);

        /* Update the grid's scale, alphafac, and possibly point data. */
        if (grid.update(camera))
            /* Only update the buffer when the point data changes. */
            glBufferData(GL_ARRAY_BUFFER, grid.points.size() * sizeof(glm::vec2), grid.points.data(), GL_DYNAMIC_DRAW);

        glVertexAttribPointer(vertex, 2, GL_FLOAT, GL_FALSE, 0, 0);

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

        /* No more buffer. */
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        /* Draw to the depth buffer again. */
        glDepthMask(GL_TRUE);
    }

    if (drawPlanarCircles) {
        /* First draw the lines from the circle center to the planet location. */
        glUniform4fv(shaderColor_color, 1, glm::value_ptr(glm::vec4(0.8f)));
        glUniformMatrix4fv(shaderColor_modelMatrix, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));

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
    glBindVertexArray(circleVAO);

    if (drawPlanarCircles) {
        /* Then we draw a circle at the XY position of every planet. */
        for (const auto& i : universe) {
            glm::vec3 pos = i.position;
            pos.z = 0;

            glm::mat4 matrix = glm::translate(pos);
            /* Make the circle start at the planet's radius and increase it slightly the further out the camera is. */
            matrix = glm::scale(matrix, glm::vec3(i.radius() * drawScale + camera.distance * 0.02f));

            glUniformMatrix4fv(shaderColor_modelMatrix, 1, GL_FALSE, glm::value_ptr(matrix));
            glDrawElements(GL_LINES, circleLineCount, GL_UNSIGNED_INT, (GLvoid*)circleLineStart);
        }
    }

    if ((placing.step == PlacingInterface::OrbitalPlane || placing.step == PlacingInterface::OrbitalPlanet)
            && universe.isSelectedValid() && placing.orbitalRadius > 0.0f) {
        glUniform4fv(shaderColor_color, 1, glm::value_ptr(glm::vec4(1.0f)));

        glm::mat4 newRadiusMatrix = placing.getOrbitalCircleMat();
        glm::mat4 oldRadiusMatrix = placing.getOrbitedCircleMat();

        /* Draw both circles. */
        glUniformMatrix4fv(shaderColor_modelMatrix, 1, GL_FALSE, glm::value_ptr(newRadiusMatrix));
        glDrawElements(GL_LINES, circleLineCount, GL_UNSIGNED_INT, (GLvoid*)circleLineStart);

        glUniformMatrix4fv(shaderColor_modelMatrix, 1, GL_FALSE, glm::value_ptr(oldRadiusMatrix));
        glDrawElements(GL_LINES, circleLineCount, GL_UNSIGNED_INT, (GLvoid*)circleLineStart);
    }

    /* If there is a controller attached, we aren't placing, and we aren't following anything, draw a little circle in the center of the screen. */
    if (gamepad.isAttached() && placing.step == PlacingInterface::NotPlacing && camera.followingState == Camera::FollowNone) {
        glDisable(GL_DEPTH_TEST);

        glUniform4fv(shaderColor_color, 1, glm::value_ptr(glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)));

        glm::mat4 matrix = glm::translate(camera.position);
        matrix = glm::scale(matrix, glm::vec3(camera.distance * 4.0e-3f));
        glUniformMatrix4fv(shaderColor_modelMatrix, 1, GL_FALSE, glm::value_ptr(matrix));

        glDrawElements(GL_LINES, circleLineCount, GL_UNSIGNED_INT, (GLvoid*)circleLineStart);

        glEnable(GL_DEPTH_TEST);
    }

    glBindVertexArray(0);
}

void PlanetsWindow::paintUI(const float delay) {
    ImGuiIO& io = ImGui::GetIO();

    io.DeltaTime = delay;

    /* Get the mouse position. */
    int mx, my;
    SDL_GetMouseState(&mx, &my);
    /* Mouse position, in pixels (set to -1,-1 if no mouse / on another screen, etc.) */
    if (SDL_GetWindowFlags(windowSDL) & SDL_WINDOW_MOUSE_FOCUS)
        io.MousePos = ImVec2((float)mx, (float)my);
    else
        io.MousePos = ImVec2(-1, -1);

    SDL_SetCursor(cursors[ImGui::GetMouseCursor()]);

    /* Hide OS cursor if imgui has one. */
    SDL_ShowCursor(io.MouseDrawCursor ? SDL_FALSE : SDL_TRUE);

    /* Begin UI code. */
    ImGui::NewFrame();

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New", "Ctrl+N"))
                newUniverse();

#ifdef PLANETS3D_WITH_NFD
            if (ImGui::MenuItem("Open", "Ctrl+O"))
                openFile();
            if (ImGui::MenuItem("Append", "Ctrl+A"))
                appendFile();

            if (ImGui::MenuItem("Save", "Ctrl+S"))
                saveFile();
#endif

            if (ImGui::MenuItem("Quit", "Escape"))
                onClose();

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Fullscreen", "Alt+Enter", fullscreen))
                toggleFullscreen();

            ImGui::Separator();

            ImGui::MenuItem("Show Grid", "Ctrl+G", &grid.draw);
            ImGui::MenuItem("Show Trails", "Ctrl+T", &drawTrails);
            ImGui::MenuItem("Show Planar Circles", "Ctrl+Y", &drawPlanarCircles);

            ImGui::Separator();

            /* All the buttons for opening the control windows... */
            ImGui::MenuItem("Planet Generator", "", &showPlanetGenWindow);
            ImGui::MenuItem("Speed Controls", "", &showSpeedWindow);
            ImGui::MenuItem("View Settings", "", &showViewSettingsWindow);
            ImGui::MenuItem("Information Window", "", &showInfoWindow);
            ImGui::MenuItem("Firing Mode Settings", "", &showFiringWindow);

            ImGui::Separator();

            if (ImGui::MenuItem("Open All Windows", ""))
                showPlanetGenWindow = showSpeedWindow = showViewSettingsWindow = showInfoWindow = showFiringWindow = true;

#ifndef NDEBUG
            ImGui::Separator();
            ImGui::MenuItem("ImGui Test Window", "", &showTestWindow);
#endif

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Create")) {
            if (ImGui::MenuItem("Interactive Creation", "Alt+P"))
                placing.beginInteractiveCreation();

            if (ImGui::MenuItem("Interactive Orbital", "Alt+O", false, universe.isSelectedValid()))
                placing.beginOrbitalCreation();

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help")) {
            ImGui::MenuItem("About", "F1", &showAboutWindow);
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if (showPlanetGenWindow) {
        ImGui::SetNextWindowPos(ImVec2(10, 30), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(360, 160), ImGuiCond_FirstUseEver);
        ImGui::Begin("Random Planet Generator", &showPlanetGenWindow);

        ImGui::Checkbox("Orbital", &planetGenOrbital);

        if (ImGui::InputInt("Amount", &planetGenAmount))
            planetGenAmount = std::max(1, planetGenAmount);

        if (!planetGenOrbital) {
            ImGui::SliderFloat("Maximum Position", &planetGenMaxPos, 1.0f, 1.0e4f, "%.3f", ImGuiSliderFlags_Logarithmic);
            ImGui::SliderFloat("Maximum Speed", &planetGenMaxSpeed, 0.0f, 200.0f);
            ImGui::SliderFloat("Maximum Mass", &planetGenMaxMass, 10.0f, 1.0e4f);
        }

        if (ImGui::Button("Generate")) {
            if (planetGenOrbital)
                universe.generateRandomOrbital(planetGenAmount, universe.selected);
            else
                universe.generateRandom(planetGenAmount, planetGenMaxPos, planetGenMaxSpeed * universe.velocityFactor, planetGenMaxMass);
        }

        ImGui::End();
    }

    if (showSpeedWindow) {
        ImGui::SetNextWindowPos(ImVec2(10, 200), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(360, 100), ImGuiCond_FirstUseEver);
        ImGui::Begin("Speed Controls", &showSpeedWindow);

        ImGui::SliderFloat("Speed", &universe.simulationSpeed, 0.0f, 64.0f, "%.3fx");

        if (ImGui::Button("Slow Down")) {
            if (universe.simulationSpeed <= 0.25f)
                universe.simulationSpeed = 0.0f;
            else
                universe.simulationSpeed *= 0.5f;
        }
        ImGui::SameLine();
        if (ImGui::Button(universe.simulationSpeed <= 0.0f ? "Resume" : "Pause")) {
            if (universe.simulationSpeed <= 0.0f)
                universe.simulationSpeed = pauseSpeed;
            else {
                pauseSpeed = universe.simulationSpeed;
                universe.simulationSpeed = 0.0f;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Fast Forward")) {
            if (universe.simulationSpeed <= 0.0f)
                universe.simulationSpeed = 0.25f;
            else if (universe.simulationSpeed < 64.0f)
                universe.simulationSpeed *= 2.0f;
        }

        ImGui::End();
    }

    if (showViewSettingsWindow) {
        ImGui::SetNextWindowPos(ImVec2(10, 310), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(360, 120), ImGuiCond_FirstUseEver);
        ImGui::Begin("View Settings", &showViewSettingsWindow);

        ImGui::SliderInt("Path Length", (int*)&universe.pathLength, 100, 4000);

        /* Show the square root of the value, because it is stored in squared units. */
        float distance = sqrt(universe.pathRecordDistance);
        if (ImGui::SliderFloat("Path Record Distance", &distance, 0.2f, 10.0f))
            universe.pathRecordDistance = distance * distance;

        ImGui::SliderInt("Steps Per Frame", &universe.stepsPerFrame, 1, 4000);
        ImGui::SliderInt("Grid Size", (int*)&grid.range, 4, 64);

        ImGui::SliderFloat("Planet Scale", &drawScale, 1.0f, 8.0f);

        ImGui::End();
    }

    if (showInfoWindow) {
        ImGui::SetNextWindowPos(ImVec2(10, 440), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(360, 320), ImGuiCond_FirstUseEver);
        ImGui::Begin("Information", &showInfoWindow);

        if (universe.isSelectedValid() && ImGui::CollapsingHeader("Selected Planet")) {
            Planet& p = universe.getSelected();
            ImGui::Text("Position: x: %f, y: %f, z: %f", p.position.x, p.position.y, p.position.z);
            ImGui::Text("Velocity: x: %f, y: %f, z: %f",
                        p.velocity.x / universe.velocityFactor, p.velocity.y / universe.velocityFactor, p.velocity.z / universe.velocityFactor);
            ImGui::Text("Mass:     %f", p.mass());
            ImGui::Text("Radius:   %f", p.radius());
            /* The materialID is a uint8_t, so it won't work directly with SliderInt(),
             * but SliderInt() just converts to a float and calls SliderFloat(), so why bother with an int in between? */
            float mat = p.materialID;
            if (ImGui::SliderFloat("Material", &mat, 0, NUM_PLANET_TEXTURES-1, "%.0f"))
                p.materialID = static_cast<uint8_t>(mat);
        }

        if (ImGui::CollapsingHeader("General Info", ImGuiTreeNodeFlags_DefaultOpen))
            ImGui::Text("Planet Count: %zu", universe.size());

        if (ImGui::CollapsingHeader("Statistics")) {
            ImGui::PlotLines("Frame Time\n(in ms)", frameTimes.data(), static_cast<int>(frameTimes.size()),
                             static_cast<int>(frameTimeOffset), nullptr, 0.0f, 100.0f, ImVec2(0.0f, 160.0f));

            ImGui::PlotLines("Frame Rate\n(in fps)",
                             [](void* data, int idx) { return 1000.0f / reinterpret_cast<float*>(data)[idx]; },
                             frameTimes.data(), static_cast<int>(frameTimes.size()),
                             static_cast<int>(frameTimeOffset), nullptr, 0.0f, FLT_MAX, ImVec2(0.0f, 120.0f));
        }

        if (ImGui::CollapsingHeader("OpenGL Info"))
            ImGui::TextWrapped(glInfo.data());

        ImGui::End();
    }

    if (showFiringWindow) {
        ImGui::SetNextWindowPos(ImVec2(10, 770), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(360, 160), ImGuiCond_FirstUseEver);
        ImGui::Begin("Firing Settings", &showFiringWindow);

        bool firingMode = placing.step == PlacingInterface::Firing;

        if (ImGui::Checkbox("Firing Mode", &firingMode))
            placing.enableFiringMode(firingMode);

        /* Show the speed in UI velocity. */
        float speed = placing.firingSpeed / universe.velocityFactor;
        if (ImGui::SliderFloat("Speed", &speed, 0.0f, 1.0e3f, "%.3f", ImGuiSliderFlags_Logarithmic))
            placing.firingSpeed = speed * universe.velocityFactor;

        ImGui::SliderFloat("Mass", &placing.firingMass, 1.0f, 1.0e3f);

        ImGui::End();
    }

    if (showAboutWindow) {
        ImGui::SetNextWindowPos(ImVec2(windowSize.x / 2, windowSize.y / 2), 0, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(400, 200));
        ImGui::Begin("About", &showAboutWindow, ImGuiWindowFlags_NoResize| ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);

        ImGui::TextWrapped("Planets3D is a simple 3D gravitational simulator");
        ImGui::NewLine();
        ImGui::TextWrapped("Website: github.com/chipgw/planets-3d");
        ImGui::NewLine();
        if (ImGui::CollapsingHeader("Build Info")) {
            ImGui::TextWrapped("  Git sha1: %s", version::git_revision);
            ImGui::TextWrapped("  Build type: %s", version::build_type);
            ImGui::TextWrapped("  Compiler: %s", version::compiler);
            ImGui::TextWrapped("  CMake Version: %s", version::cmake_version);
        }

        ImGui::End();
    }

#ifndef NDEBUG
    if (showTestWindow) {
        ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
        ImGui::ShowDemoWindow();
    }
#endif
    /* End UI code. */

    /* Begin rendering code. */

    /* Setup render state: no face culling, no depth testing, scissor enabled. */
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glActiveTexture(GL_TEXTURE0);

    /* Setup orthographic projection matrix. */
    const glm::mat4 projection( 2.0f/io.DisplaySize.x, 0.0f,                   0.0f, 0.0f,
                                0.0f,                  2.0f/-io.DisplaySize.y, 0.0f, 0.0f,
                                0.0f,                  0.0f,                  -1.0f, 0.0f,
                               -1.0f,                  1.0f,                   0.0f, 1.0f);

    glUseProgram(shaderUI);
    glUniformMatrix4fv(shaderUI_matrix, 1, GL_FALSE, glm::value_ptr(projection));

    glEnableVertexAttribArray(uv);
    glEnableVertexAttribArray(tangent);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    /* Let ImGui take all the info above and make the draw data. */
    ImGui::Render();

    ImDrawData* drawData = ImGui::GetDrawData();
    
    drawData->ScaleClipRects(io.DisplayFramebufferScale);

    int fb_width = static_cast<int>(io.DisplaySize.x * io.DisplayFramebufferScale.x);
    int fb_height = static_cast<int>(io.DisplaySize.y * io.DisplayFramebufferScale.y);
    if (fb_width == 0 || fb_height == 0)
        return;
    glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);

    constexpr GLenum indexType = sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;

    for (int n = 0; n < drawData->CmdListsCount; ++n) {
        const ImDrawList* cmdList = drawData->CmdLists[n];
        int idxBufferOffset = 0;

        glVertexAttribPointer(vertex,   2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), &cmdList->VtxBuffer.front().pos);
        glVertexAttribPointer(uv,       2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), &cmdList->VtxBuffer.front().uv);
        glVertexAttribPointer(tangent,  4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), &cmdList->VtxBuffer.front().col);

        for (const ImDrawCmd* pcmd = cmdList->CmdBuffer.begin(); pcmd != cmdList->CmdBuffer.end(); ++pcmd) {
            if (pcmd->UserCallback) {
                pcmd->UserCallback(cmdList, pcmd);
            } else {
                glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(reinterpret_cast<intptr_t>(pcmd->TextureId)));
                glScissor(static_cast<int>(pcmd->ClipRect.x), static_cast<int>(fb_height - pcmd->ClipRect.w),
                          static_cast<int>(pcmd->ClipRect.z - pcmd->ClipRect.x), static_cast<int>(pcmd->ClipRect.w - pcmd->ClipRect.y));
                glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(pcmd->ElemCount), indexType, &cmdList->IdxBuffer[idxBufferOffset]);
            }
            idxBufferOffset += pcmd->ElemCount;
        }
    }

    /* End rendering code. */

    /* Restore GL settings to the defaults that everything else uses. */
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);

    /* Make sure the viewport is as it was before rendering. */
    glViewport(0, 0, windowSize.x, windowSize.y);
}

void PlanetsWindow::toggleFullscreen() {
    fullscreen = !fullscreen;

    SDL_SetWindowFullscreen(windowSDL, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}

/* On the SDL side, 0 is invalid, 1 is LEFT, 2 is MIDDLE, 3 is RIGHT, 4 is X1, 5 is X2. */
constexpr int BTN_MAP[] = { 0, 0, 2, 1, 3, 4 };

void PlanetsWindow::doEvents() {
    ImGuiIO& io = ImGui::GetIO();
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            onClose();
            break;
        case SDL_KEYDOWN:
            /* If ImGui wants the keyboard, we just send the event, otherwise we use it. */
            if (!io.WantCaptureKeyboard)
                doKeyPress(event.key.keysym);
        case SDL_KEYUP:
            io.KeysDown[event.key.keysym.sym & ~SDLK_SCANCODE_MASK] = (event.type == SDL_KEYDOWN);
            io.KeyShift = (SDL_GetModState() & KMOD_SHIFT) != 0;
            io.KeyCtrl = (SDL_GetModState() & KMOD_CTRL) != 0;
            io.KeyAlt = (SDL_GetModState() & KMOD_ALT) != 0;
            io.KeySuper = (SDL_GetModState() & KMOD_GUI) != 0;
            break;
        case SDL_TEXTINPUT:
            io.AddInputCharactersUTF8(event.text.text);
            break;
        case SDL_WINDOWEVENT:
            switch(event.window.event) {
            case SDL_WINDOWEVENT_FOCUS_LOST:
                if (universe.simulationSpeed > 0.0f) {
                    pauseSpeed = universe.simulationSpeed;
                    universe.simulationSpeed = 0.0f;
                }
                break;
            case SDL_WINDOWEVENT_FOCUS_GAINED:
                if (pauseSpeed > 0.0f && universe.simulationSpeed <= 0.0f)
                    universe.simulationSpeed = pauseSpeed;
                break;
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
            /* If ImGui wants the mouse, we ignore it for placing & camera purposes. */
            if (!io.WantCaptureMouse && !placing.handleMouseWheel(event.wheel.y * 0.2f)) {
                camera.distance -= event.wheel.y * camera.distance * 0.1f;
                camera.zrotation -= event.wheel.x * 0.05f;

                camera.bound();
            }

            /* Pass to ImGui. */
            if (event.wheel.y > 0)
                io.MouseWheel = 1;
            if (event.wheel.y < 0)
                io.MouseWheel = -1;
            break;
        case SDL_MOUSEBUTTONUP:
            /* If ImGui wants the mouse, we ignore it here. */
            if (!io.WantCaptureMouse) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    if (event.button.clicks == 2 && placing.step == PlacingInterface::NotPlacing) {
                        if (universe.isSelectedValid())
                            camera.followSelection();
                        else
                            camera.clearFollow();
                    } else {
                        glm::ivec2 pos(event.button.x, event.button.y);
                        if (!placing.handleMouseClick(pos, camera))
                            camera.selectUnder(pos, drawScale);
                    }
                } else if ((event.button.button == SDL_BUTTON_MIDDLE || event.button.button == SDL_BUTTON_RIGHT) && event.button.clicks == 2) {
                    camera.reset();
                }
            }

            /* Always show cursor when mouse button is released. */
            SDL_SetRelativeMouseMode(SDL_FALSE);

            /* Continue on to pass event to ImGui. */
        case SDL_MOUSEBUTTONDOWN:
            io.MouseDown[BTN_MAP[event.button.button]] = event.type == SDL_MOUSEBUTTONDOWN;
            break;
        case SDL_DROPFILE:
            try {
                universe.load(event.drop.file);
            } catch (std::exception e) {
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Unable to load file dropped on window", e.what(), windowSDL);
            }

            SDL_free(event.drop.file);
            break;
        case SDL_MOUSEMOTION:
            /* yrel is inverted compared to the delta calculated in Qt. */
            glm::ivec2 delta(event.motion.xrel, -event.motion.yrel);

            bool holdCursor = false;

            if (!io.WantCaptureMouse && !placing.handleMouseMove(glm::ivec2(event.motion.x, event.motion.y), delta, camera, holdCursor)) {
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
        if (key.mod & KMOD_ALT)
            placing.beginInteractiveCreation();
        break;
    case SDLK_o:
        if (key.mod & KMOD_ALT)
            placing.beginOrbitalCreation();
#ifdef PLANETS3D_WITH_NFD
        if (key.mod & KMOD_CTRL)
            openFile();
        break;
    case SDLK_s:
        if (key.mod & KMOD_CTRL)
            saveFile();
        break;
    case SDLK_a:
        if (key.mod & KMOD_CTRL)
            appendFile();
#endif
        break;
    case SDLK_c:
        if (key.mod & KMOD_ALT)
            universe.centerAll();
        break;
    case SDLK_DELETE:
        universe.deleteSelected();
        break;
    case SDLK_RETURN:
        if (key.mod & KMOD_ALT)
            toggleFullscreen();
        break;
    case SDLK_n:
        if (key.mod & KMOD_CTRL)
            newUniverse();
        break;
    case SDLK_t:
        if (key.mod & KMOD_CTRL)
            drawTrails = !drawTrails;
        break;
    case SDLK_y:
        if (key.mod & KMOD_CTRL)
            drawPlanarCircles = !drawPlanarCircles;
        break;
    case SDLK_g:
        if (key.mod & KMOD_CTRL)
            grid.toggle();
        break;
    case SDLK_F1:
        showAboutWindow = !showAboutWindow;
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

    /* Tell ImGui what the viewportsize is. */
    io.DisplaySize = ImVec2(static_cast<float>(width), static_cast<float>(height));

    /* This has something to do with high DPI displays... */
    int display_w, display_h;
    SDL_GL_GetDrawableSize(windowSDL, &display_w, &display_h);
    io.DisplayFramebufferScale = ImVec2(width > 0 ? (static_cast<float>(display_w) / static_cast<float>(width)) : 0,
                                        height > 0 ? (static_cast<float>(display_h) / static_cast<float>(height)) : 0);
}

void PlanetsWindow::drawPlanetWireframe(const Planet& planet, const glm::vec4& color) {
    glUniform4fv(shaderColor_color, 1, glm::value_ptr(color));

    glm::mat4 matrix = glm::translate(planet.position);
    matrix = glm::scale(matrix, glm::vec3(planet.radius() * drawScale * 1.05f));
    glUniformMatrix4fv(shaderColor_modelMatrix, 1, GL_FALSE, glm::value_ptr(matrix));

    glDrawElements(GL_LINES, lowResLineCount, GL_UNSIGNED_INT, (GLvoid*)lowResLineStart);
}
