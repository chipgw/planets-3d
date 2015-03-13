project(Planets3D_sdl)
cmake_minimum_required(VERSION 3.1)

option(PLANETS3D_SDL "Build SDL Interface Executable" OFF)

if(PLANETS3D_SDL)
    if(WIN32)
        set(PLANETS3D_WITH_GLEW ON)
    else(WIN32)
        option(PLANETS3D_WITH_GLEW "Use GLEW to load OpenGL functions." OFF)
    endif(WIN32)

    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

    file(GLOB SDL_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
    file(GLOB SDL_SHADERS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*")

    file(GLOB SDL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

    add_executable(${PROJECT_NAME} WIN32 ${SDL_SOURCES} ${SDL_HEADERS} ${SDL_WINDRES} ${SDL_SHADERS})

    find_package(SDL2 REQUIRED)
    include_directories(${SDL2_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY})

    if(MSVC)
        target_link_libraries(${PROJECT_NAME} ${SDL2MAIN_LIBRARY})
    endif(MSVC)

    find_package(SDL2_image REQUIRED)
    include_directories(${SDL2_IMAGE_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${SDL2_IMAGE_LIBRARY})

    if(PLANETS3D_WITH_GLEW)
        add_definitions(-DPLANETS3D_WITH_GLEW)
        find_package(GLEW REQUIRED)
        include_directories(${GLEW_INCLUDE_DIR})
        target_link_libraries(${PROJECT_NAME} ${GLEW_LIBRARIES})
    endif(PLANETS3D_WITH_GLEW)

    find_package(OpenGL REQUIRED)
    include_directories(${OPENGL_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} Planets3D ${OPENGL_gl_LIBRARY})

    add_custom_command(TARGET ${PROJECT_NAME} COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/textures/planet.png
                                                                               ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/)
endif(PLANETS3D_SDL)
