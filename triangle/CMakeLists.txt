cmake_minimum_required(VERSION 2.8)

set(PROJECT_NAME triangle)

project(${PROJECT_NAME})

set (CMAKE_BUILD_TYPE Release)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_FLAGS "-Wall -Wextra")
set (CMAKE_CXX_FLAGS_DEBUG "-g")
set (CMAKE_CXX_FLAGS_RELEASE "-O3")

set(include_path "/usr/include/GL /usr/include/glm")
set(lib_path "/usr/lib")

INCLUDE_DIRECTORIES(${include_path})
LINK_DIRECTORIES(${lib_path})
add_executable(${PROJECT_NAME} main.cpp)

#copy all glsl files to build folder
file(GLOB_RECURSE GLSL_FILES "*.glsl")
foreach(glsl_file ${GLSL_FILES})
    configure_file(${glsl_file} . COPYONLY)
endforeach(glsl_file)

TARGET_LINK_LIBRARIES(${PROJECT_NAME} GLEW)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} glfw)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} GL)
