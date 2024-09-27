cmake_minimum_required(VERSION 3.5)

project(ShaderEditor)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Gui Core)
qt_standard_project_setup()
find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)

message(STATUS "Vulkan found: ${Vulkan_FOUND}")
message(STATUS "Vulkan include directory: ${Vulkan_INCLUDE_DIR}")
message(STATUS "Vulkan library: ${Vulkan_LIBRARIES}")

include_directories(${CMAKE_SOURCE_DIR}/Include/ThirdParty/GLFW)
include_directories(${CMAKE_SOURCE_DIR}/Source)
include_directories(${glfw3_DIR}/include)
include_directories(${Vulkan_INCLUDE_DIR})

set(HEADER_FILES
    Source/Core/SeVulkanManager.h
    Source/Core/SeQueueFamilyIndices.h
    Source/Core/SeVulkanWindow.h
)

set(SRC_FILES
    Source/main.cpp

    Source/Core/SeVulkanManager.cpp
    Source/Core/SeVulkanWindow.cpp
)

add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SRC_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE glfw ${Vulkan_LIBRARIES} Qt6::Core Qt6::Widgets Qt6::Gui)
