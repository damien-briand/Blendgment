cmake_minimum_required(VERSION 3.20)
project(Blendgment VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# -- Dependencies -------------------------------------------------------------
find_package(Vulkan REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(CURL   REQUIRED)
find_package(LibArchive REQUIRED)

# -- ImGUI via FetchContent ---------------------------------------------------
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

# Lien statique des runtimes MinGW pour imgui_lib (Windows/GCC uniquement)
if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_options(imgui_lib PUBLIC
        -static-libgcc
        -static-libstdc++
    )
endif()

# -- Project sources ----------------------------------------------------------
set(SOURCES
    src/main.cpp
    src/Application.cpp
    src/VulkanContext.cpp
    src/UI/UIManager.cpp
    src/UI/InstalledVersion.cpp
)
set(UI_PAGES
    src/UI/pages/DashboardPage.cpp
    src/UI/pages/VersionsPage.cpp
    src/UI/pages/ProjectsPage.cpp
    src/UI/pages/SettingsPage.cpp
)
set(UI_MODALS
    src/UI/modals/InstallModal.cpp
    src/UI/modals/DeleteModal.cpp
    src/UI/modals/NewProjectModal.cpp
    src/UI/modals/AddAssetModal.cpp
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

set(WIN_RESOURCES
    resources/app_icon.rc
)

add_executable(${PROJECT_NAME}
    ${SOURCES} ${UI_PAGES} ${UI_MODALS} ${CORE} ${RENDERING} ${SERVICES} ${UTILS}
    $<$<PLATFORM_ID:Windows>:${WIN_RESOURCES}>
)

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

# -- Definitions Windows ------------------------------------------------------
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        _CRT_SECURE_NO_WARNINGS
        NOMINMAX
        WIN32_LEAN_AND_MEAN
    )
    # Lien statique des runtimes MinGW (GCC) pour eviter libgcc / libwinpthread
    # manquants sur les machines sans MinGW installe.
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_link_options(${PROJECT_NAME} PRIVATE
            -static-libgcc
            -static-libstdc++
            -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive,-Bdynamic
        )
    endif()
    # Copie automatique des DLLs vcpkg a cote de l'executable
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND_EXPAND_LISTS
    )
endif()

# -- Flags de compilation -----------------------------------------------------
target_compile_options(${PROJECT_NAME} PRIVATE
    # GCC / Clang
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra>
    $<$<AND:$<CXX_COMPILER_ID:GNU,Clang>,$<CONFIG:Debug>>:   -g -O0>
    $<$<AND:$<CXX_COMPILER_ID:GNU,Clang>,$<CONFIG:Release>>: -O2 -DNDEBUG>
    # MSVC
    $<$<CXX_COMPILER_ID:MSVC>:/W3 /wd4996 /utf-8>
    $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Debug>>:   /Od /Zi>
    $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>: /O2 /DNDEBUG>
)

message(STATUS "Vulkan found : ${Vulkan_VERSION}")
message(STATUS "Build type   : ${CMAKE_BUILD_TYPE}")
message(STATUS "Platform     : ${CMAKE_SYSTEM_NAME}")
