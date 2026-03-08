#pragma once

#include <vulkan/vulkan.h>
#include <vector>

// ─────────────────────────────────────────────────────────────────────────────
// Renderer  –  command pool/buffers, synchronisation, descriptor pool
//              + per-frame acquire / submit / present cycle
// ─────────────────────────────────────────────────────────────────────────────
class Renderer {
public:
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    Renderer(VkDevice   device,
             VkQueue    graphicsQueue,
             VkQueue    presentQueue,
             uint32_t   graphicsQueueFamily,
             uint32_t   imageCount);
    ~Renderer();

    Renderer(const Renderer&)            = delete;
    Renderer& operator=(const Renderer&) = delete;

    // ── Frame operations ──────────────────────────────────────────────────────
    // Returns false → swapchain out-of-date, caller should recreate
    bool            beginFrame    (VkSwapchainKHR swapChain, uint32_t& outImageIndex);
    VkCommandBuffer beginCommandBuffer();
    void            beginRenderPass(VkCommandBuffer cmd, uint32_t imageIndex,
                                    VkRenderPass    renderPass,
                                    VkFramebuffer   framebuffer,
                                    VkExtent2D      extent);
    void            endRenderPass  (VkCommandBuffer cmd);
    void            endCommandBuffer(VkCommandBuffer cmd);
    // Returns false → swapchain out-of-date
    bool            submitAndPresent(VkCommandBuffer cmd, uint32_t imageIndex,
                                     VkSwapchainKHR  swapChain);

    // Call after swapchain recreation to reset per-image fence tracking
    void onSwapChainRecreated(uint32_t newImageCount);

    // ── Single-time command utility ───────────────────────────────────────────
    VkCommandBuffer beginSingleTimeCommands();
    void            endSingleTimeCommands(VkCommandBuffer cmd);

    // ── Getters ───────────────────────────────────────────────────────────────
    VkCommandPool    getCommandPool()    const { return m_commandPool; }
    VkDescriptorPool getDescriptorPool() const { return m_descriptorPool; }
    uint32_t         getCurrentFrame()  const { return m_currentFrame; }

private:
    void createCommandPool(uint32_t graphicsFamily);
    void createCommandBuffers();
    void createSyncObjects(uint32_t imageCount);
    void createDescriptorPool();

    VkDevice m_device;
    VkQueue  m_graphicsQueue;
    VkQueue  m_presentQueue;

    VkCommandPool                m_commandPool    = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> m_commandBuffers;
    VkDescriptorPool             m_descriptorPool = VK_NULL_HANDLE;

    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence>     m_inFlightFences;
    std::vector<VkFence>     m_imagesInFlight;   // indexed by swapchain image index

    uint32_t m_currentFrame = 0;
};
