#include "Application.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <stdexcept>
#include <iostream>

Application::Application()
{
    m_window = std::make_unique<Window>(WIDTH, HEIGHT, "Blendgment");
    m_vulkan = std::make_unique<VulkanContext>(
        m_window->getNativeWindow(), WIDTH, HEIGHT);
    m_ui     = std::make_unique<UIManager>();

    initImGui();
    std::cout << "[App] Initialisation complete.\n";
}

Application::~Application()
{
    m_vulkan->waitIdle();
    shutdownImGui();
}

// ─────────────────────────────────────────────────────────────────────────────
// ImGUI init / shutdown
// ─────────────────────────────────────────────────────────────────────────────
void Application::initImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename  = "blendgment.ini";

    // ── General Style ─────────────────────────────────────────────────────────
    ImGui::StyleColorsDark();
    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowRounding    = 0.f;
    s.FrameRounding     = 5.f;
    s.ScrollbarRounding = 5.f;
    s.GrabRounding      = 5.f;
    s.TabRounding       = 5.f;
    s.WindowBorderSize  = 0.f;
    s.FrameBorderSize   = 0.f;
    s.ItemSpacing       = ImVec2(8.f, 6.f);
    s.WindowPadding     = ImVec2(0.f, 0.f);

    // ── Color ────────────────────────────────────
    ImVec4* c = s.Colors;
    c[ImGuiCol_WindowBg]          = {0.11f, 0.11f, 0.14f, 1.f};
    c[ImGuiCol_ChildBg]           = {0.00f, 0.00f, 0.00f, 0.f};
    c[ImGuiCol_FrameBg]           = {0.20f, 0.20f, 0.26f, 1.f};
    c[ImGuiCol_FrameBgHovered]    = {0.26f, 0.26f, 0.34f, 1.f};
    c[ImGuiCol_FrameBgActive]     = {0.22f, 0.22f, 0.29f, 1.f};
    c[ImGuiCol_TitleBgActive]     = {0.10f, 0.10f, 0.13f, 1.f};
    c[ImGuiCol_CheckMark]         = {0.87f, 0.45f, 0.10f, 1.f};
    c[ImGuiCol_SliderGrab]        = {0.87f, 0.45f, 0.10f, 1.f};
    c[ImGuiCol_SliderGrabActive]  = {0.96f, 0.56f, 0.16f, 1.f};
    c[ImGuiCol_Button]            = {0.22f, 0.22f, 0.28f, 1.f};
    c[ImGuiCol_ButtonHovered]     = {0.87f, 0.45f, 0.10f, 1.f};
    c[ImGuiCol_ButtonActive]      = {0.68f, 0.33f, 0.06f, 1.f};
    c[ImGuiCol_Header]            = {0.87f, 0.45f, 0.10f, 0.40f};
    c[ImGuiCol_HeaderHovered]     = {0.87f, 0.45f, 0.10f, 0.60f};
    c[ImGuiCol_HeaderActive]      = {0.87f, 0.45f, 0.10f, 1.00f};
    c[ImGuiCol_Separator]         = {0.28f, 0.28f, 0.36f, 1.f};
    c[ImGuiCol_ResizeGrip]        = {0.87f, 0.45f, 0.10f, 0.25f};
    c[ImGuiCol_ResizeGripHovered] = {0.87f, 0.45f, 0.10f, 0.67f};
    c[ImGuiCol_ResizeGripActive]  = {0.87f, 0.45f, 0.10f, 0.95f};
    c[ImGuiCol_Tab]               = {0.16f, 0.16f, 0.20f, 1.f};
    c[ImGuiCol_TabHovered]        = {0.87f, 0.45f, 0.10f, 0.80f};
    c[ImGuiCol_TabActive]         = {0.87f, 0.45f, 0.10f, 1.00f};
    c[ImGuiCol_ScrollbarBg]       = {0.10f, 0.10f, 0.13f, 1.f};
    c[ImGuiCol_ScrollbarGrab]     = {0.28f, 0.28f, 0.36f, 1.f};
    c[ImGuiCol_ScrollbarGrabHovered]= {0.38f, 0.38f, 0.48f, 1.f};
    c[ImGuiCol_PopupBg]           = {0.16f, 0.16f, 0.20f, 0.96f};

    // ── Backend GLFW ──────────────────────────────────────────────────────────
    ImGui_ImplGlfw_InitForVulkan(m_window->getNativeWindow(), true);

    // ── Backend Vulkan ────────────────────────────────────────────────────────
    ImGui_ImplVulkan_InitInfo vkInit{};
    vkInit.ApiVersion     = VK_API_VERSION_1_2;
    vkInit.Instance       = m_vulkan->getInstance();
    vkInit.PhysicalDevice = m_vulkan->getPhysicalDevice();
    vkInit.Device         = m_vulkan->getDevice();
    vkInit.QueueFamily    = m_vulkan->getGraphicsQueueFamily();
    vkInit.Queue          = m_vulkan->getGraphicsQueue();
    vkInit.DescriptorPool = m_vulkan->getDescriptorPool();
    vkInit.MinImageCount  = 2;
    vkInit.ImageCount     = m_vulkan->getImageCount();

    vkInit.PipelineInfoMain.RenderPass  = m_vulkan->getRenderPass();
    vkInit.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&vkInit);
}

void Application::shutdownImGui()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Application::run()
{
    while (!m_window->shouldClose()) {
        m_window->pollEvents();
        renderFrame();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Resize Management
// ─────────────────────────────────────────────────────────────────────────────
void Application::handleResize()
{
    int w = 0, h = 0;
    glfwGetFramebufferSize(m_window->getNativeWindow(), &w, &h);
    // Attendre si la fenêtre est minimisée
    while (w == 0 || h == 0) {
        glfwGetFramebufferSize(m_window->getNativeWindow(), &w, &h);
        glfwWaitEvents();
    }
    m_vulkan->recreateSwapChain(w, h);
    ImGui_ImplVulkan_SetMinImageCount(2);
    m_window->resetResizeFlag();
}

// ─────────────────────────────────────────────────────────────────────────────
// Frame rendering
// ─────────────────────────────────────────────────────────────────────────────
void Application::renderFrame()
{
    // ── 1. Check Resize ─────────────────────────────────────────────────
    if (m_window->wasResized()) {
        handleResize();
        return;
    }

    // ── 2. Swapchain ─────────────────────────────────────────
    uint32_t imageIndex = 0;
    if (!m_vulkan->beginFrame(imageIndex)) {
        handleResize();
        return;
    }

    // ── 3. New frame ImGUI ───────────────────────────────────────────────
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    m_ui->render();

    ImGui::Render();
    ImDrawData* drawData = ImGui::GetDrawData();

    // ── 4. Save Image in Commande Buffer ────────────────────────────────
    VkCommandBuffer cmd = m_vulkan->beginCommandBuffer();
    m_vulkan->beginRenderPass(cmd, imageIndex);

    ImGui_ImplVulkan_RenderDrawData(drawData, cmd);

    m_vulkan->endRenderPass(cmd);
    m_vulkan->endCommandBuffer(cmd);

    // ── 5. Render ──────────────────────────────────────────
    if (!m_vulkan->submitAndPresent(cmd, imageIndex)) {
        handleResize();
    }
}
