cmake_minimum_required(VERSION 3.5)
project(main)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)

message(STATUS "Vulkan found: ${Vulkan_FOUND}")
message(STATUS "Vulkan include directory: ${Vulkan_INCLUDE_DIR}")
message(STATUS "Vulkan library: ${Vulkan_LIBRARIES}")

include_directories(${CMAKE_SOURCE_DIR}/Include/ThirdParty/GLFW)
include_directories(${glfw3_DIR}/include)
include_directories(${Vulkan_INCLUDE_DIR})

set(SRC_FILES
    Source/main.cpp
)

add_executable(${PROJECT_NAME} ${SRC_FILES})
target_link_libraries(${PROJECT_NAME} glfw ${Vulkan_LIBRARIES})