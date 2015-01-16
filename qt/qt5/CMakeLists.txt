project(Planets3D_qt5)
cmake_minimum_required(VERSION 2.8.11)

find_package(Qt5OpenGL)

qt5_add_resources(QT_RESOURCES_RCC ${QT_RESOURCES})
qt5_wrap_ui(QT_FORM_CODE ${QT_FORMS})

add_executable(${PROJECT_NAME} WIN32 ${QT_SOURCES} ${QT_FORM_CODE} ${QT_HEADERS} ${QT_RESOURCES_RCC} ${QT_WINDRES} ${QT_SHADERS})

target_link_libraries(${PROJECT_NAME} Qt5::OpenGL Planets3D)

if(DEFINED Qt5Gui_OPENGL_LIBRARIES)
    include_directories(${Qt5Gui_OPENGL_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${Qt5Gui_OPENGL_LIBRARIES})
else(DEFINED Qt5Gui_OPENGL_LIBRARIES)
    target_link_libraries(${PROJECT_NAME} ${OPENGL_gl_LIBRARY})
endif(DEFINED Qt5Gui_OPENGL_LIBRARIES)
