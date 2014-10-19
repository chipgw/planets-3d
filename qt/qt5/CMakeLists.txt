project(Planets3D_qt5)
cmake_minimum_required(VERSION 2.8.11)

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

qt5_add_resources(QT_RESOURCES_RCC ${QT_RESOURCES})
qt5_wrap_ui(QT_FORM_CODE ${QT_FORMS})

if(PLANETS3D_DISABLE_QOPENGLTEXTURE)
    add_definitions(-DPLANETS3D_DISABLE_QOPENGLTEXTURE)
endif(PLANETS3D_DISABLE_QOPENGLTEXTURE)

add_executable(${PROJECT_NAME} WIN32 ${QT_SOURCES} ${QT_FORM_CODE} ${QT_HEADERS} ${QT_RESOURCES_RCC} ${QT_WINDRES} ${QT_SHADERS})

qt5_use_modules(${PROJECT_NAME} OpenGL)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} Qt5::WinMain)
endif(WIN32)

if(DEFINED Qt5Gui_OPENGL_LIBRARIES)
    include_directories(${Qt5Gui_OPENGL_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${Qt5Gui_OPENGL_LIBRARIES} Planets3D)
else(DEFINED Qt5Gui_OPENGL_LIBRARIES)
    target_link_libraries(${PROJECT_NAME} ${OPENGL_gl_LIBRARY} Planets3D)
endif(DEFINED Qt5Gui_OPENGL_LIBRARIES)
