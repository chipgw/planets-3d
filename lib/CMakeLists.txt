project(Planets3D)
cmake_minimum_required(VERSION 3.1)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

# files not needed in build, but included so they show up in IDE.
file(GLOB LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")

file(GLOB LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/version.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/version.cpp" @ONLY)
list(APPEND LIB_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/version.cpp")

add_library(${PROJECT_NAME} SHARED ${LIB_SOURCES} ${LIB_HEADERS})

find_package(TinyXML REQUIRED)
include_directories(${TINYXML_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${TINYXML_LIBRARIES})
