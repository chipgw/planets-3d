set(CMAKE_BUILD_TYPE_INIT "Debug")

project(Planets3D)
cmake_minimum_required(VERSION 2.8)

option(PLANETS3D_USE_QT4 "use Qt 4 rather than Qt 5." OFF)
option(PLANETS3D_DISABLE_QOPENGLTEXTURE "Don't use QOpenGLTexture even if the system supports it" OFF)

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

file(GLOB SOURCES "src/*.cpp")
file(GLOB FORMS "forms/*.ui")
set(RESOURCES "resources.qrc")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/version.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/version.cpp" @ONLY)
list(APPEND SOURCES "${CMAKE_CURRENT_BINARY_DIR}/version.cpp")

if(WIN32)
    if(${CMAKE_GENERATOR} MATCHES "Visual Studio*")
        add_definitions(-DPLANETS3D_MSVC_CONFIGURATION="$(ConfigurationName)")
    endif()

    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Planets3D.rc.in" "${CMAKE_CURRENT_BINARY_DIR}/Planets3D.rc" @ONLY)
    set(WINDRES "${CMAKE_CURRENT_BINARY_DIR}/Planets3D.rc")
else(WIN32)
    set(WINDRES "")
endif(WIN32)

if(PLANETS3D_USE_QT4)
    find_package(Qt4 4.8 REQUIRED QtCore QtGui QtOpenGL)
    include(${QT_USE_FILE})

    qt4_add_resources(RESOURCES_RCC ${RESOURCES})
    qt4_wrap_ui(FORM_HEADERS ${FORMS})
else(PLANETS3D_USE_QT4)
    find_package(Qt5OpenGL)
    include_directories(${Qt5OpenGL_INCLUDE_DIRS})
    add_definitions(${Qt5OpenGL_DEFINITIONS})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5OpenGL_EXECUTABLE_COMPILE_FLAGS}")

    qt5_wrap_ui(FORM_HEADERS ${FORMS})
    qt5_add_resources(RESOURCES_RCC ${RESOURCES})

    if(PLANETS3D_DISABLE_QOPENGLTEXTURE)
        add_definitions(-DPLANETS3D_DISABLE_QOPENGLTEXTURE)
    endif(PLANETS3D_DISABLE_QOPENGLTEXTURE)
endif(PLANETS3D_USE_QT4)

add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${FORM_HEADERS} ${HEADERS} ${RESOURCES_RCC} ${WINDRES} ${SHADERS})

if(PLANETS3D_USE_QT4)
    qt4_use_modules(${PROJECT_NAME} Core Gui OpenGL)
else(PLANETS3D_USE_QT4)
    qt5_use_modules(${PROJECT_NAME} OpenGL)

    if(WIN32)
        target_link_libraries(${PROJECT_NAME} Qt5::WinMain)
    endif(WIN32)
endif(PLANETS3D_USE_QT4)

if(DEFINED Qt5Gui_OPENGL_LIBRARIES)
    include_directories(${Qt5Gui_OPENGL_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${Qt5Gui_OPENGL_LIBRARIES})
else(DEFINED Qt5Gui_OPENGL_LIBRARIES)
    find_package(OpenGL REQUIRED)
    include_directories(${OPENGL_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${OPENGL_gl_LIBRARY})
endif(DEFINED Qt5Gui_OPENGL_LIBRARIES)
