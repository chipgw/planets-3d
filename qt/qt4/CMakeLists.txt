project(Planets3D_qt4)
cmake_minimum_required(VERSION 2.8)

find_package(Qt4 4.8 REQUIRED QtCore QtGui QtOpenGL)
include(${QT_USE_FILE})

qt4_add_resources(QT_RESOURCES_RCC ${QT_RESOURCES})
qt4_wrap_ui(QT_FORM_CODE ${QT_FORMS})

add_executable(${PROJECT_NAME} WIN32 ${QT_SOURCES} ${QT_FORM_CODE} ${QT_HEADERS} ${QT_RESOURCES_RCC} ${QT_WINDRES} ${QT_SHADERS})

qt4_use_modules(${PROJECT_NAME} Core Gui OpenGL)

target_link_libraries(${PROJECT_NAME} Planets3D ${OPENGL_gl_LIBRARY})
