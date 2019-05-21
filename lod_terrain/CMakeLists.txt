cmake_minimum_required(VERSION 2.8)

set(PROJECT_NAME lod_terrain)

project(${PROJECT_NAME})

set (CMAKE_BUILD_TYPE Release)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_FLAGS "-Wall -Wextra")
set (CMAKE_CXX_FLAGS_DEBUG "-g")
set (CMAKE_CXX_FLAGS_RELEASE "-O3")

set(INCLUDE_PATH "/usr/include/GL" "/usr/include/glm" "${CMAKE_CURRENT_SOURCE_DIR}/../common")
set(LIB_PATH "/usr/lib")

INCLUDE_DIRECTORIES(${INCLUDE_PATH})
LINK_DIRECTORIES(${LIB_PATH})

file(GLOB SOURCE_FILES "*.cpp" "*.h")
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

#copy all glsl files to build folder
file(GLOB_RECURSE GLSL_FILES "*.glsl")

foreach(glsl_file ${GLSL_FILES})
    configure_file(${glsl_file} . COPYONLY)
endforeach(glsl_file)

#configure_file(${GLSL_FILES} . COPYONLY)


TARGET_LINK_LIBRARIES(${PROJECT_NAME} GLEW)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} glfw)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} GL)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} IL)
