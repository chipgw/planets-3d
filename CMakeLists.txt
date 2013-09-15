project(Planets3D)
cmake_minimum_required(VERSION 2.8)

option(PLANETS3D_USE_QT4 "use Qt 4 rather than Qt 5." OFF)

set(Planets3D_VERSION_MAJOR 0)
set(Planets3D_VERSION_MINOR 4)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/")

include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/version.cpp.in" "${CMAKE_CURRENT_SOURCE_DIR}/src/version.cpp" @ONLY)

file(GLOB HEADERS "include/*.h")
file(GLOB SOURCES "src/*.cpp")
file(GLOB FORMS "forms/*.ui")
set(RESOURCES "resources.qrc")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Planets3D.rc.in" "${CMAKE_CURRENT_SOURCE_DIR}/Planets3D.rc" @ONLY)
set(WINDRES "Planets3D.rc")

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
endif(PLANETS3D_USE_QT4)

add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${FORM_HEADERS} ${HEADERS} ${RESOURCES_RCC} ${WINDRES})

if(PLANETS3D_USE_QT4)
    qt4_use_modules(${PROJECT_NAME} Core Gui OpenGL)
else(PLANETS3D_USE_QT4)
    qt5_use_modules(${PROJECT_NAME} OpenGL)
endif(PLANETS3D_USE_QT4)

if(DEFINED Qt5Gui_OPENGL_LIBRARIES)
    include_directories(${Qt5Gui_OPENGL_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${Qt5Gui_OPENGL_LIBRARIES})
else(DEFINED Qt5Gui_OPENGL_LIBRARIES)
    find_package(OpenGL REQUIRED)
    include_directories(${OPENGL_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${OPENGL_gl_LIBRARY})
endif(DEFINED Qt5Gui_OPENGL_LIBRARIES)
