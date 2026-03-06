#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <functional>
#include <string>

// ─────────────────────────────────────────────────────────────────────────────
// Window  –  wraps a GLFW window and its resize state
// ─────────────────────────────────────────────────────────────────────────────
class Window {
public:
    Window(int width, int height, const std::string& title);
    ~Window();

    Window(const Window&)            = delete;
    Window& operator=(const Window&) = delete;

    // ── Accessors ─────────────────────────────────────────────────────────────
    bool        shouldClose()       const { return glfwWindowShouldClose(m_window); }
    void        pollEvents()        const { glfwPollEvents(); }
    GLFWwindow* getNativeWindow()   const { return m_window; }
    int         getWidth()          const { return m_width; }
    int         getHeight()         const { return m_height; }

    // ── Resize tracking ───────────────────────────────────────────────────────
    bool wasResized()         const { return m_resized; }
    void resetResizeFlag()          { m_resized = false; }

    // Optional user callback
    void setResizeCallback(std::function<void(int, int)> cb) { m_resizeCallback = cb; }

private:
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

    GLFWwindow*                   m_window  = nullptr;
    int                           m_width   = 0;
    int                           m_height  = 0;
    std::string                   m_title;
    bool                          m_resized = false;
    std::function<void(int, int)> m_resizeCallback;
};
