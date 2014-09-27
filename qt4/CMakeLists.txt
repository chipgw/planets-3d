project(Planets3D_qt4)
cmake_minimum_required(VERSION 2.8)

find_package(Qt4 4.8 REQUIRED QtCore QtGui QtOpenGL)
include(${QT_USE_FILE})

file(GLOB QT_SHADERS "${Planets3D_SOURCE_DIR}/shaders/*")

file(GLOB QT_SOURCES "${Planets3D_SOURCE_DIR}/src/qt/*.cpp")
file(GLOB FORMS "${Planets3D_SOURCE_DIR}/forms/*.ui")
set(RESOURCES "${Planets3D_SOURCE_DIR}/resources.qrc")

qt4_add_resources(RESOURCES_RCC ${RESOURCES})
qt4_wrap_ui(FORM_HEADERS ${FORMS})

add_executable(${PROJECT_NAME} WIN32 ${QT_SOURCES} ${FORM_HEADERS} ${HEADERS} ${RESOURCES_RCC} ${WINDRES} ${QT_SHADERS})

qt4_use_modules(${PROJECT_NAME} Core Gui OpenGL)

target_link_libraries(${PROJECT_NAME} Planets3D ${TINYXML_LIBRARIES} ${OPENGL_gl_LIBRARY})
