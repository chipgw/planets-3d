set(CMAKE_BUILD_TYPE_INIT "Debug")

project(Planets3D)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin CACHE PATH "Directory for libraries")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin CACHE PATH "Directory for executables.")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin CACHE PATH "Directory for static libraries.")

option(PLANETS3D_QT4 "Build Qt4 Interface Executable" OFF)
option(PLANETS3D_QT5 "Build Qt5 Interface Executable" OFF)
# TODO - Make SDL interface.
# option(PLANETS3D_SDL "Build SDL Interface Executable" OFF)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    # add any future MSVC related stuff here.
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)

# files not needed in build, but included so they show up in IDE.
file(GLOB HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB SHADERS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*")

file(GLOB LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/lib/*.cpp")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/version.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/version.cpp" @ONLY)
list(APPEND LIB_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/version.cpp")

add_library(${PROJECT_NAME} ${LIB_SOURCES})

if(WIN32)
    if(${CMAKE_GENERATOR} MATCHES "Visual Studio*")
        add_definitions(-DPLANETS3D_MSVC_CONFIGURATION="$(ConfigurationName)")
    endif()

    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Planets3D.rc.in" "${CMAKE_CURRENT_BINARY_DIR}/Planets3D.rc" @ONLY)
    set(WINDRES "${CMAKE_CURRENT_BINARY_DIR}/Planets3D.rc")
else(WIN32)
    set(WINDRES "")
endif(WIN32)

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

find_package(TinyXML REQUIRED)
include_directories(${TINYXML_INCLUDE_DIRS})

if(PLANETS3D_QT4)
    add_subdirectory(qt4)
endif(PLANETS3D_QT4)

if(PLANETS3D_QT5)
    add_subdirectory(qt5)
endif(PLANETS3D_QT5)
