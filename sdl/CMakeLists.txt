project(Planets3D_sdl)
cmake_minimum_required(VERSION 2.8)

option(PLANETS3D_SDL "Build SDL Interface Executable" OFF)

if(PLANETS3D_SDL)
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

    file(GLOB SDL_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
    file(GLOB SDL_SHADERS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*")

    file(GLOB SDL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

    add_executable(${PROJECT_NAME} WIN32 ${SDL_SOURCES} ${SDL_HEADERS} ${SDL_WINDRES} ${SDL_SHADERS})

    find_package(SDL2 REQUIRED)
    include_directories(${SDL2_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY})

    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        target_link_libraries(${PROJECT_NAME} ${SDL2MAIN_LIBRARY})
    endif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")

    find_package(SDL2_image REQUIRED)
    include_directories(${SDL2_IMAGE_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${SDL2_IMAGE_LIBRARY})

    target_link_libraries(${PROJECT_NAME} Planets3D ${OPENGL_gl_LIBRARY})
endif(PLANETS3D_SDL)
