set(CMAKE_BUILD_TYPE_INIT "Debug")

project(Planets3D)
cmake_minimum_required(VERSION 3.2)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin CACHE PATH "Directory for libraries")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin CACHE PATH "Directory for executables.")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin CACHE PATH "Directory for static libraries.")

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/lib/include" "${CMAKE_CURRENT_SOURCE_DIR}/lib" "${CMAKE_CURRENT_SOURCE_DIR}/include")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

# Retrieve the current git sha1 for use inthe configured version.cpp.
include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)

# Macro to copy a file orfolder to the directory where binaries are built to.
macro(copy_to_bin)
    make_directory("${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/${ARGV2}")
    add_custom_command(TARGET ${ARGV0} COMMAND ${CMAKE_COMMAND} -E copy "${ARGV1}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/${ARGV2}")
endmacro()

# Options for native builds.
if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
    option(PLANETS3D_QT5 "Build Qt Interface Executable" OFF)
    option(PLANETS3D_SDL "Build SDL Interface Executable" OFF)

    if(PLANETS3D_QT5)
        option(PLANETS3D_QT_USE_SDL_GAMEPAD "Use SDL library for gamepad support in Qt executable." OFF)
    endif(PLANETS3D_QT5)

    option(PLANETS3D_BUILD_TINYXML "Look for TinyXML source files in \"tinyxml\" folder, and build as a static library." OFF)
    option(PLANETS3D_BENCHMARK "Build a command-line program to test simulation performance without any graphics." OFF)

    # Visual Studio projects have multiple build configurations in one generated project file...
    if(${CMAKE_GENERATOR} MATCHES "Visual Studio*")
        add_definitions(-DPLANETS3D_MSVC_CONFIGURATION="$(ConfigurationName)")
    endif()
endif(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")

find_package(GLM REQUIRED)
include_directories(${GLM_INCLUDE_DIR})

#---------------------------------------------
# Library
#---------------------------------------------

# files not needed in build, but included so they show up in IDE.
file(GLOB LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/lib/include/*.h")

file(GLOB LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/lib/src/*.cpp")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/lib/src/version.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/version.cpp" @ONLY)
list(APPEND LIB_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/version.cpp" README.md LICENSE)

if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
    # If we're building for HTML we just throw everything into one project later, otherwise we use a shared library for this.
    add_library(${PROJECT_NAME} SHARED ${LIB_SOURCES} ${LIB_HEADERS})

    if(PLANETS3D_BUILD_TINYXML)
        # Build TinyXML from source files in the tinyxml folder.
        add_definitions(-DTIXML_USE_STL)
        file(GLOB TinyXML_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/tinyxml/*.cpp")
        add_library(TinyXML ${TinyXML_SOURCES})
        include_directories("${CMAKE_CURRENT_SOURCE_DIR}/tinyxml/")
        target_link_libraries(${PROJECT_NAME} TinyXML)
    else(PLANETS3D_BUILD_TINYXML)
        # Locate the prebuilt version of TinyXML. (Can be very fussy on Windows.)
        find_package(TinyXML REQUIRED)
        include_directories(${TINYXML_INCLUDE_DIR})
        target_link_libraries(${PROJECT_NAME} ${TINYXML_LIBRARIES})
    endif(PLANETS3D_BUILD_TINYXML)

    if(PLANETS3D_BENCHMARK)
        add_executable(${PROJECT_NAME}_benchmark "bench/bench.cpp")
        target_link_libraries(${PROJECT_NAME}_benchmark ${PROJECT_NAME})
    endif(PLANETS3D_BENCHMARK)

    # The SDL gamepad support works with both interfaces from the same source code,
    # but it doesn't go in the library because that doesn't require SDL.
    if(PLANETS3D_SDL OR PLANETS3D_QT_USE_SDL_GAMEPAD)
        find_package(SDL2 REQUIRED)
        include_directories(${SDL2_INCLUDE_DIR} "gamepad")

        add_library(${PROJECT_NAME}_gamepad STATIC "gamepad/sdlgamepad.h" "gamepad/sdlgamepad.cpp")
    endif(PLANETS3D_SDL OR PLANETS3D_QT_USE_SDL_GAMEPAD)
endif(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")

#---------------------------------------------
# Emscripten Interface
#---------------------------------------------

if(${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
    # Already set, but Qt Creator doesn't know about it.
    add_definitions(-DEMSCRIPTEN)

    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/gamepad")

    # Still needs SDL for gamepad support.
    set(CMAKE_CXX_FLAGS "--bind -s FULL_ES2=1 -s USE_SDL=2 -std=c++11")
    set(CMAKE_CXX_FLAGS_DEBUG "-s DEMANGLE_SUPPORT=1 -g")

    # Included for IDE support.
    file(GLOB JS_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/js/*.*" "${CMAKE_CURRENT_SOURCE_DIR}/js/*/*.*" )

    add_executable(${PROJECT_NAME}_js ${LIB_SOURCES} ${LIB_HEADERS} ${JS_SOURCES} "gamepad/sdlgamepad.h" "gamepad/sdlgamepad.cpp" "bench/bench.cpp")

    # All these files that the JS interface needs...
    # TODO - Find a way to make them copy again any time they change.
    copy_to_bin(${PROJECT_NAME}_js "${CMAKE_SOURCE_DIR}/textures/*" "images/")
    copy_to_bin(${PROJECT_NAME}_js "${CMAKE_SOURCE_DIR}/js/scripts/*" "scripts/")
    copy_to_bin(${PROJECT_NAME}_js "${CMAKE_SOURCE_DIR}/js/scripts/thirdparty/*" "scripts/thirdparty/")
    copy_to_bin(${PROJECT_NAME}_js $<TARGET_FILE:${PROJECT_NAME}_js> "scripts/")
    copy_to_bin(${PROJECT_NAME}_js "${CMAKE_SOURCE_DIR}/js/stylesheets/*" "stylesheets/")
    copy_to_bin(${PROJECT_NAME}_js "${CMAKE_SOURCE_DIR}/js/systems/*" "systems/")
    copy_to_bin(${PROJECT_NAME}_js "${CMAKE_SOURCE_DIR}/js/index.html")
endif(${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")

#---------------------------------------------
# SDL Interface
#---------------------------------------------

if(PLANETS3D_SDL)
    # Places resource files into a .cpp and .hpp file. (stolen from stackoverflow with some tweaks)
    function(CREATE_RESOURCES OUTFILE)
        set(OUTHPP "${CMAKE_CURRENT_BINARY_DIR}/${OUTFILE}.hpp")
        set(OUTCPP "${CMAKE_CURRENT_BINARY_DIR}/${OUTFILE}.cpp")

        # Create output files.
        file(WRITE ${OUTHPP} "#pragma once\n#include <cstddef>\n")
        file(WRITE ${OUTCPP} "#include \"${OUTFILE}.hpp\"\n")

        # Iterate through input files
        foreach(FILE ${ARGN})
            # Get short filename
            string(REGEX MATCH "([^/]+)$" FILENAME ${FILE})
            # Replace filename spaces & extension separator for C compatibility
            string(REGEX REPLACE "\\.| |-" "_" FILENAME ${FILENAME})
            # Read hex data from file
            file(READ ${FILE} FILEDATA HEX)
            # Convert hex data for C compatibility
            string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," FILEDATA ${FILEDATA})
            # Append data to output file
            file(APPEND ${OUTHPP} "extern const unsigned char ${FILENAME}[];\nextern const size_t ${FILENAME}_size;\n")
            file(APPEND ${OUTCPP} "const unsigned char ${FILENAME}[] = {${FILEDATA}0x00};\nconst size_t ${FILENAME}_size = sizeof(${FILENAME});\n")
        endforeach()
    endfunction()

    if(WIN32)
        # GLEW is required on Windows.
        set(PLANETS3D_WITH_GLEW ON)
    else(WIN32)
        # Elsewhere including glcorearb.h usually works, but we leave the possibility of using GLEW open.
        option(PLANETS3D_WITH_GLEW "Use GLEW to load OpenGL functions. (SDL interface only)" OFF)
    endif(WIN32)

    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/sdl/include")

    # For IDE usage.
    file(GLOB SDL_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/sdl/include/*.h")

    # For the resource system and IDE usage.
    file(GLOB SDL_SHADERS "${CMAKE_CURRENT_SOURCE_DIR}/sdl/shaders/*")

    file(GLOB SDL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/sdl/src/*.cpp")

    file(GLOB IMGUI_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/imgui/*.cpp")

    # Use that resource generation script to generate res.hpp and res.cpp.
    create_resources(res ${SDL_SHADERS} "${CMAKE_SOURCE_DIR}/textures/planet_diffuse.png" "${CMAKE_SOURCE_DIR}/textures/planet_nrm.png")

    add_executable(${PROJECT_NAME}_sdl WIN32 ${SDL_SOURCES} ${IMGUI_SOURCES} ${SDL_HEADERS} ${SDL_WINDRES} ${SDL_SHADERS} res.cpp)

    if(MSVC)
        # MSVC needs to link to SDL's main wrapper.
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
endif(PLANETS3D_SDL)

#---------------------------------------------
# Qt Interface
#---------------------------------------------

if(PLANETS3D_QT5)
    # Find all the source files. Again, headers are included to show up in IDE.
    file(GLOB QT_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/qt/include/*.h")
    file(GLOB QT_SHADERS "${CMAKE_CURRENT_SOURCE_DIR}/qt/shaders/*")

    file(GLOB QT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/qt/src/*.cpp")
    file(GLOB QT_FORMS "${CMAKE_CURRENT_SOURCE_DIR}/qt/forms/*.ui")
    set(QT_RESOURCES "${CMAKE_CURRENT_SOURCE_DIR}/qt/resources.qrc")

    if(WIN32)
        # For the windows exe icon and version information.
        set(QT_WINDRES "${CMAKE_CURRENT_BINARY_DIR}/qt/Planets3D.rc")
        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/qt/Planets3D.rc.in" "${QT_WINDRES}" @ONLY)
    else(WIN32)
        # Nothing replaces it on other platforms.
        set(QT_WINDRES "")
    endif(WIN32)

    find_package(Qt5OpenGL)
    set(CMAKE_AUTOMOC ON)

    # Use Qt's tools to generate resource and UI code.
    qt5_add_resources(QT_RESOURCES_RCC ${QT_RESOURCES})
    qt5_wrap_ui(QT_FORM_CODE ${QT_FORMS})

    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/qt/include" ${Qt5OpenGL_INCLUDE_DIRS})
    add_executable(${PROJECT_NAME}_qt WIN32 ${QT_SOURCES} ${QT_FORM_CODE} ${QT_HEADERS} ${QT_RESOURCES_RCC} ${QT_WINDRES} ${QT_SHADERS})

    target_link_libraries(${PROJECT_NAME}_qt Qt5::OpenGL Planets3D)

    if(PLANETS3D_QT_USE_SDL_GAMEPAD)
        # Link to the SDL gamepad code and enable it.
        target_link_libraries(${PROJECT_NAME}_qt ${PROJECT_NAME}_gamepad ${SDL2_LIBRARY})
        add_definitions(-DPLANETS3D_QT_USE_SDL_GAMEPAD)
    endif(PLANETS3D_QT_USE_SDL_GAMEPAD)
endif(PLANETS3D_QT5)
