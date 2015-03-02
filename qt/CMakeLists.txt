project(Planets3D_qt5)
cmake_minimum_required(VERSION 2.8.11)

unset(PLANETS3D_QT4 CACHE)
option(PLANETS3D_QT5 "Build Qt5 Interface Executable" OFF)

file(GLOB QT_HEADERS "${Planets3D_qt_SOURCE_DIR}/include/*.h")
file(GLOB QT_SHADERS "${Planets3D_qt_SOURCE_DIR}/shaders/*")

file(GLOB QT_SOURCES "${Planets3D_qt_SOURCE_DIR}/src/*.cpp")
file(GLOB QT_FORMS "${Planets3D_qt_SOURCE_DIR}/forms/*.ui")
set(QT_RESOURCES "${Planets3D_qt_SOURCE_DIR}/resources.qrc")

if(WIN32)
    set(QT_WINDRES "${CMAKE_CURRENT_BINARY_DIR}/Planets3D.rc")
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Planets3D.rc.in" "${QT_WINDRES}" @ONLY)
else(WIN32)
    set(QT_WINDRES "")
endif(WIN32)

find_package(Qt5OpenGL)
set(CMAKE_AUTOMOC ON)

qt5_add_resources(QT_RESOURCES_RCC ${QT_RESOURCES})
qt5_wrap_ui(QT_FORM_CODE ${QT_FORMS})

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
add_executable(${PROJECT_NAME} WIN32 ${QT_SOURCES} ${QT_FORM_CODE} ${QT_HEADERS} ${QT_RESOURCES_RCC} ${QT_WINDRES} ${QT_SHADERS})

target_link_libraries(${PROJECT_NAME} Qt5::OpenGL Planets3D)

if(DEFINED Qt5Gui_OPENGL_LIBRARIES)
    include_directories(${Qt5Gui_OPENGL_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${Qt5Gui_OPENGL_LIBRARIES})
else(DEFINED Qt5Gui_OPENGL_LIBRARIES)
    target_link_libraries(${PROJECT_NAME} ${OPENGL_gl_LIBRARY})
endif(DEFINED Qt5Gui_OPENGL_LIBRARIES)
