set(CMAKE_BUILD_TYPE_INIT "Debug")

project(Planets3D)
cmake_minimum_required(VERSION 3.2)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin CACHE PATH "Directory for libraries")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin CACHE PATH "Directory for executables.")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin CACHE PATH "Directory for static libraries.")

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

option(PLANETS3D_QT5 "Build Qt Interface Executable" OFF)
option(PLANETS3D_SDL "Build SDL Interface Executable" OFF)
option(PLANETS3D_QT_USE_SDL_GAMEPAD "Use SDL library for gamepad support in Qt executable." OFF)
option(PLANETS3D_BUILD_TINYXML "Look for TinyXML source files in \"tinyxml\" folder, and build as a static library." OFF)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/lib/include" "${CMAKE_CURRENT_SOURCE_DIR}/lib" "${CMAKE_CURRENT_SOURCE_DIR}/include")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)

if(${CMAKE_GENERATOR} MATCHES "Visual Studio*")
    add_definitions(-DPLANETS3D_MSVC_CONFIGURATION="$(ConfigurationName)")
endif()

find_package(GLM REQUIRED)
include_directories(${GLM_INCLUDE_DIR})

#---------------------------------------------
# Library
#---------------------------------------------

# files not needed in build, but included so they show up in IDE.
file(GLOB LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/lib/include/*.h")

file(GLOB LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/lib/src/*.cpp")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/lib/src/version.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/version.cpp" @ONLY)
list(APPEND LIB_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/version.cpp")

add_library(${PROJECT_NAME} SHARED ${LIB_SOURCES} ${LIB_HEADERS} README.md LICENSE)

if(PLANETS3D_BUILD_TINYXML)
    add_definitions(-DTIXML_USE_STL)
    file(GLOB TinyXML_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/tinyxml/*.cpp")
    add_library(TinyXML ${TinyXML_SOURCES})
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/tinyxml/")
    target_link_libraries(${PROJECT_NAME} TinyXML)
else(PLANETS3D_BUILD_TINYXML)
    find_package(TinyXML REQUIRED)
    include_directories(${TINYXML_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${TINYXML_LIBRARIES})
endif(PLANETS3D_BUILD_TINYXML)

if(PLANETS3D_SDL OR PLANETS3D_QT_USE_SDL_GAMEPAD)
    find_package(SDL2 REQUIRED)
    include_directories(${SDL2_INCLUDE_DIR} "gamepad")

    add_library(${PROJECT_NAME}_gamepad STATIC "gamepad/sdlgamepad.h" "gamepad/sdlgamepad.cpp")
endif(PLANETS3D_SDL OR PLANETS3D_QT_USE_SDL_GAMEPAD)

#---------------------------------------------
# SDL Interface
#---------------------------------------------

if(PLANETS3D_SDL)
    if(WIN32)
        set(PLANETS3D_WITH_GLEW ON)
    else(WIN32)
        option(PLANETS3D_WITH_GLEW "Use GLEW to load OpenGL functions. (SDL interface only)" OFF)
    endif(WIN32)

    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/sdl/include")

    file(GLOB SDL_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/sdl/include/*.h")
    file(GLOB SDL_SHADERS "${CMAKE_CURRENT_SOURCE_DIR}/sdl/shaders/*")

    file(GLOB SDL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/sdl/src/*.cpp")

    add_executable(${PROJECT_NAME}_sdl WIN32 ${SDL_SOURCES} ${SDL_HEADERS} ${SDL_WINDRES} ${SDL_SHADERS})

    if(MSVC)
        target_link_libraries(${PROJECT_NAME}_sdl ${SDL2_SDLMAIN_LIBRARY})
    endif(MSVC)

    find_package(SDL2_image REQUIRED)
    include_directories(${SDL2_IMAGE_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME}_sdl ${SDL2_IMAGE_LIBRARY})

    if(PLANETS3D_WITH_GLEW)
        add_definitions(-DPLANETS3D_WITH_GLEW)
        find_package(GLEW REQUIRED)
        include_directories(${GLEW_INCLUDE_DIR})
        target_link_libraries(${PROJECT_NAME}_sdl ${GLEW_LIBRARIES})
    endif(PLANETS3D_WITH_GLEW)

    find_package(OpenGL REQUIRED)
    include_directories(${OPENGL_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME}_sdl ${SDL2_LIBRARY} ${PROJECT_NAME}_gamepad ${PROJECT_NAME} ${OPENGL_gl_LIBRARY})

    add_custom_command(TARGET ${PROJECT_NAME}_sdl COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/textures/planet.png
                                                                                   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/)
endif(PLANETS3D_SDL)

#---------------------------------------------
# Qt Interface
#---------------------------------------------

if(PLANETS3D_QT5)
    file(GLOB QT_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/qt/include/*.h")
    file(GLOB QT_SHADERS "${CMAKE_CURRENT_SOURCE_DIR}/qt/shaders/*")

    file(GLOB QT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/qt/src/*.cpp")
    file(GLOB QT_FORMS "${CMAKE_CURRENT_SOURCE_DIR}/qt/forms/*.ui")
    set(QT_RESOURCES "${CMAKE_CURRENT_SOURCE_DIR}/qt/resources.qrc")

    if(WIN32)
        set(QT_WINDRES "${CMAKE_CURRENT_BINARY_DIR}/qt/Planets3D.rc")
        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/qt/Planets3D.rc.in" "${QT_WINDRES}" @ONLY)
    else(WIN32)
        set(QT_WINDRES "")
    endif(WIN32)

    find_package(Qt5OpenGL)
    set(CMAKE_AUTOMOC ON)

    qt5_add_resources(QT_RESOURCES_RCC ${QT_RESOURCES})
    qt5_wrap_ui(QT_FORM_CODE ${QT_FORMS})

    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/qt/include" ${Qt5OpenGL_INCLUDE_DIRS})
    add_executable(${PROJECT_NAME}_qt WIN32 ${QT_SOURCES} ${QT_FORM_CODE} ${QT_HEADERS} ${QT_RESOURCES_RCC} ${QT_WINDRES} ${QT_SHADERS})

    target_link_libraries(${PROJECT_NAME}_qt Qt5::OpenGL Planets3D)

    if(PLANETS3D_QT_USE_SDL_GAMEPAD)
        target_link_libraries(${PROJECT_NAME}_qt ${PROJECT_NAME}_gamepad ${SDL2_LIBRARY})
        add_definitions(-DPLANETS3D_QT_USE_SDL_GAMEPAD)
    endif(PLANETS3D_QT_USE_SDL_GAMEPAD)
endif(PLANETS3D_QT5)
