cmake_minimum_required(VERSION 3.20)
project(Blendgment VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# ── Dependencies ──────────────────────────────────────────────────────────────
find_package(Vulkan REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(CURL   REQUIRED)
find_package(LibArchive REQUIRED)

# ── ImGUI via FetchContent ────────────────────────────────────────────────────
include(FetchContent)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.92.6
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(imgui)

set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
)

add_library(imgui_lib STATIC ${IMGUI_SOURCES})
target_include_directories(imgui_lib PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${Vulkan_INCLUDE_DIRS}
)
target_link_libraries(imgui_lib PUBLIC Vulkan::Vulkan glfw)

# ── Project sources ───────────────────────────────────────────────────────────
set(SOURCES
    src/main.cpp
    src/Application.cpp
    src/VulkanContext.cpp
    src/UI/UIManager.cpp
)

set(CORE
    src/core/Window.cpp
    src/core/Instance.cpp
    src/core/Device.cpp
)

set(RENDERING
    src/rendering/SwapChain.cpp
    src/rendering/Renderer.cpp
)

set(SERVICES
    src/services/BlenderFetcher.cpp
)

set(UTILS
    src/utils/SynchronousDownloader.cpp
    src/utils/Extractor.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES} ${CORE} ${RENDERING} ${SERVICES} ${UTILS})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    imgui_lib
    Vulkan::Vulkan
    glfw
    CURL::libcurl
    LibArchive::LibArchive
)

# ── Compiler flags ────────────────────────────────────────────────────────────
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra>
    $<$<CONFIG:Debug>:  -g -O0>
    $<$<CONFIG:Release>:-O2 -DNDEBUG>
)

message(STATUS "Vulkan found : ${Vulkan_VERSION}")
message(STATUS "Build type   : ${CMAKE_BUILD_TYPE}")
