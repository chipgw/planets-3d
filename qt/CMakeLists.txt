project(Planets3D_qt)
cmake_minimum_required(VERSION 2.8)

option(PLANETS3D_QT4 "Build Qt4 Interface Executable" OFF)
option(PLANETS3D_QT5 "Build Qt5 Interface Executable" OFF)

set(CMAKE_AUTOMOC ON)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

file(GLOB QT_HEADERS "${Planets3D_qt_SOURCE_DIR}/include/*.h")
file(GLOB QT_SHADERS "${Planets3D_qt_SOURCE_DIR}/shaders/*")

file(GLOB QT_SOURCES "${Planets3D_qt_SOURCE_DIR}/src/*.cpp")
file(GLOB QT_FORMS "${Planets3D_qt_SOURCE_DIR}/forms/*.ui")
set(QT_RESOURCES "${Planets3D_qt_SOURCE_DIR}/resources.qrc")


if(PLANETS3D_QT4)
    add_subdirectory(qt4)
endif(PLANETS3D_QT4)

if(PLANETS3D_QT5)
    add_subdirectory(qt5)
endif(PLANETS3D_QT5)
