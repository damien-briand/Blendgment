#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "core/Window.h"
#include "VulkanContext.h"
#include "UI/UIManager.h"

#include <memory>

class Application {
public:
    Application();
    ~Application();

    Application(const Application&)            = delete;
    Application& operator=(const Application&) = delete;

    void run();

private:
    void initImGui();
    void shutdownImGui();
    void renderFrame();
    void handleResize();

    std::unique_ptr<Window>        m_window;
    std::unique_ptr<VulkanContext> m_vulkan;
    std::unique_ptr<UIManager>     m_ui;

    static constexpr int WIDTH  = 1280;
    static constexpr int HEIGHT = 800;
};
