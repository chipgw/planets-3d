project(Planets3D_qt5)
cmake_minimum_required(VERSION 2.8)

option(PLANETS3D_DISABLE_QOPENGLTEXTURE "Don't use QOpenGLTexture even if the system supports it" OFF)

# Silences warnings about the policy not being set.
# We still manually link to Qt5::WinMain to support older versions of CMake.
# As far as I can tell there is no harm in doing both.
if(POLICY CMP0020)
    cmake_policy(SET CMP0020 NEW)
endif(POLICY CMP0020)

find_package(Qt5OpenGL)
include_directories(${Qt5OpenGL_INCLUDE_DIRS})
add_definitions(${Qt5OpenGL_DEFINITIONS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5OpenGL_EXECUTABLE_COMPILE_FLAGS}")

file(GLOB QT_SHADERS "${Planets3D_SOURCE_DIR}/shaders/*")

file(GLOB QT_SOURCES "${Planets3D_SOURCE_DIR}/src/qt/*.cpp")
file(GLOB FORMS "${Planets3D_SOURCE_DIR}/forms/*.ui")
set(RESOURCES "${Planets3D_SOURCE_DIR}/resources.qrc")

qt5_add_resources(RESOURCES_RCC ${RESOURCES})
qt5_wrap_ui(FORM_HEADERS ${FORMS})

if(PLANETS3D_DISABLE_QOPENGLTEXTURE)
    add_definitions(-DPLANETS3D_DISABLE_QOPENGLTEXTURE)
endif(PLANETS3D_DISABLE_QOPENGLTEXTURE)

add_executable(${PROJECT_NAME} WIN32 ${QT_SOURCES} ${FORM_HEADERS} ${HEADERS} ${RESOURCES_RCC} ${WINDRES} ${QT_SHADERS})

qt5_use_modules(${PROJECT_NAME} OpenGL)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} Qt5::WinMain)
endif(WIN32)

if(DEFINED Qt5Gui_OPENGL_LIBRARIES)
    include_directories(${Qt5Gui_OPENGL_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${Qt5Gui_OPENGL_LIBRARIES})
else(DEFINED Qt5Gui_OPENGL_LIBRARIES)
    target_link_libraries(${PROJECT_NAME} ${OPENGL_gl_LIBRARY})
endif(DEFINED Qt5Gui_OPENGL_LIBRARIES)

target_link_libraries(${PROJECT_NAME} Planets3D ${TINYXML_LIBRARIES})
