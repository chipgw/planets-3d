set(CMAKE_BUILD_TYPE_INIT "Debug")

project(Planets3D)
cmake_minimum_required(VERSION 2.8)

option(PLANETS3D_QT4 "Build Qt4 Interface Executable" OFF)
option(PLANETS3D_QT5 "Build Qt5 Interface Executable" OFF)
# TODO - Make SDL interface.
# option(PLANETS3D_SDL "Build SDL Interface Executable" OFF)

if(PLANETS3D_QT4 AND PLANETS3D_QT5)
    # TODO - Fix this.
    message(FATAL_ERROR "Currently it is not possible to enable building both Qt interfaces in the same build directory due to automoc limitations,"
                        " this will be fixed later. For now only either PLANETS3D_QT4 or PLANETS3D_QT5.")
endif(PLANETS3D_QT4 AND PLANETS3D_QT5)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    # add any future MSVC related stuff here.
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

# Silences warnings about the policy not being set.
# We still manually link to Qt5::WinMain to support older versions of CMake.
# As far as I can tell there is no harm in doing both.
if(POLICY CMP0020)
    cmake_policy(SET CMP0020 NEW)
endif(POLICY CMP0020)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)

# files not needed in build, but included so they show up in IDE.
file(GLOB HEADERS "include/*.h")
file(GLOB SHADERS "shaders/*")

file(GLOB LIB_SOURCES "src/lib/*.cpp")
file(GLOB QT_SOURCES "src/qt/*.cpp")
file(GLOB FORMS "forms/*.ui")
set(RESOURCES "resources.qrc")

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
    find_package(Qt4 4.8 REQUIRED QtCore QtGui QtOpenGL)
    include(${QT_USE_FILE})

    qt4_add_resources(RESOURCES_RCC ${RESOURCES})
    qt4_wrap_ui(FORM_HEADERS ${FORMS})

    add_executable(${PROJECT_NAME}_qt4 WIN32 ${QT_SOURCES} ${FORM_HEADERS} ${HEADERS} ${RESOURCES_RCC} ${WINDRES} ${SHADERS})

    qt4_use_modules(${PROJECT_NAME}_qt4 Core Gui OpenGL)

    target_link_libraries(${PROJECT_NAME}_qt4 ${PROJECT_NAME} ${TINYXML_LIBRARIES} ${OPENGL_gl_LIBRARY})
endif(PLANETS3D_QT4)

if(PLANETS3D_QT5)
    option(PLANETS3D_DISABLE_QOPENGLTEXTURE "Don't use QOpenGLTexture even if the system supports it" OFF)
    find_package(Qt5OpenGL)
    include_directories(${Qt5OpenGL_INCLUDE_DIRS})
    add_definitions(${Qt5OpenGL_DEFINITIONS})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5OpenGL_EXECUTABLE_COMPILE_FLAGS}")

    qt5_wrap_ui(FORM_HEADERS ${FORMS})
    qt5_add_resources(RESOURCES_RCC ${RESOURCES})

    if(PLANETS3D_DISABLE_QOPENGLTEXTURE)
        add_definitions(-DPLANETS3D_DISABLE_QOPENGLTEXTURE)
    endif(PLANETS3D_DISABLE_QOPENGLTEXTURE)

    add_executable(${PROJECT_NAME}_qt5 WIN32 ${QT_SOURCES} ${FORM_HEADERS} ${HEADERS} ${RESOURCES_RCC} ${WINDRES} ${SHADERS})

    qt5_use_modules(${PROJECT_NAME}_qt5 OpenGL)

    if(WIN32)
        target_link_libraries(${PROJECT_NAME}_qt5 Qt5::WinMain)
    endif(WIN32)
    if(DEFINED Qt5Gui_OPENGL_LIBRARIES)
        include_directories(${Qt5Gui_OPENGL_INCLUDE_DIRS})
        target_link_libraries(${PROJECT_NAME}_qt5 ${Qt5Gui_OPENGL_LIBRARIES})
    else(DEFINED Qt5Gui_OPENGL_LIBRARIES)
        target_link_libraries(${PROJECT_NAME}_qt5 ${OPENGL_gl_LIBRARY})
    endif(DEFINED Qt5Gui_OPENGL_LIBRARIES)

    target_link_libraries(${PROJECT_NAME}_qt5 ${PROJECT_NAME} ${TINYXML_LIBRARIES})
endif(PLANETS3D_QT5)
