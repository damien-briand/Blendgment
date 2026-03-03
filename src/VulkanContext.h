#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <optional>
#include <string>
#include <vector>

// ─────────────────────────────────────────────────────────────────────────────
// Helper structs
// ─────────────────────────────────────────────────────────────────────────────
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    bool isComplete() const { return graphicsFamily.has_value() && presentFamily.has_value(); }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR        capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR>   presentModes;
};

// ─────────────────────────────────────────────────────────────────────────────
// VulkanContext  –  owns all Vulkan objects and drives the render loop
//
//  Frame flow (driven by Application):
//    1.  bool ok = beginFrame(imageIndex)
//    2.  VkCommandBuffer cmd = beginCommandBuffer()
//    3.  beginRenderPass(cmd, imageIndex)
//    4.  record draw calls / ImGUI
//    5.  endRenderPass(cmd)
//    6.  endCommandBuffer(cmd)
//    7.  submitAndPresent(cmd, imageIndex)
// ─────────────────────────────────────────────────────────────────────────────
class VulkanContext {
public:
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    VulkanContext(GLFWwindow* window, int width, int height);
    ~VulkanContext();

    VulkanContext(const VulkanContext&)            = delete;
    VulkanContext& operator=(const VulkanContext&) = delete;

    // ── Getters used by Application / ImGUI init ──────────────────────────────
    VkInstance        getInstance()          const { return m_instance; }
    VkDevice          getDevice()            const { return m_device; }
    VkPhysicalDevice  getPhysicalDevice()    const { return m_physicalDevice; }
    VkRenderPass      getRenderPass()        const { return m_renderPass; }
    VkCommandPool     getCommandPool()       const { return m_commandPool; }
    VkQueue           getGraphicsQueue()     const { return m_graphicsQueue; }
    VkDescriptorPool  getDescriptorPool()    const { return m_descriptorPool; }
    uint32_t          getImageCount()        const { return static_cast<uint32_t>(m_swapChainImages.size()); }
    VkExtent2D        getSwapChainExtent()   const { return m_swapChainExtent; }
    uint32_t          getGraphicsQueueFamily() const { return m_queueFamilyIndices.graphicsFamily.value(); }
    uint32_t          getCurrentFrame()      const { return m_currentFrame; }

    // ── Frame operations ──────────────────────────────────────────────────────
    // Returns false if the swapchain is out-of-date (caller should recreate)
    bool             beginFrame(uint32_t& outImageIndex);
    VkCommandBuffer  beginCommandBuffer();
    void             beginRenderPass(VkCommandBuffer cmd, uint32_t imageIndex);
    void             endRenderPass(VkCommandBuffer cmd);
    void             endCommandBuffer(VkCommandBuffer cmd);
    // Returns false when the swapchain must be recreated (OUT_OF_DATE / SUBOPTIMAL)
    bool             submitAndPresent(VkCommandBuffer cmd, uint32_t imageIndex);

    // ── Swapchain recreation ──────────────────────────────────────────────────
    void recreateSwapChain(int width, int height);

    // ── Utility ──────────────────────────────────────────────────────────────
    VkCommandBuffer beginSingleTimeCommands();
    void            endSingleTimeCommands(VkCommandBuffer cmd);
    void            waitIdle() const;

private:
    // ── Init helpers ──────────────────────────────────────────────────────────
    void createInstance();
    void setupDebugMessenger();
    void createSurface(GLFWwindow* window);
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain(int width, int height);
    void createImageViews();
    void createRenderPass();     // called ONCE in constructor
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffers();
    void createSyncObjects();
    void createDescriptorPool(); // large pool for ImGUI

    // Destroy only swapchain-dependent objects (render pass is kept alive)
    void cleanupSwapChain();

    // ── Device selection helpers ──────────────────────────────────────────────
    bool                    checkValidationLayerSupport();
    std::vector<const char*>getRequiredExtensions();
    bool                    isDeviceSuitable(VkPhysicalDevice device);
    bool                    checkDeviceExtensionSupport(VkPhysicalDevice device);
    QueueFamilyIndices      findQueueFamilies(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    VkSurfaceFormatKHR      chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
    VkPresentModeKHR        chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& modes);
    VkExtent2D              chooseSwapExtent(const VkSurfaceCapabilitiesKHR& caps, int w, int h);

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT,
        VkDebugUtilsMessageTypeFlagsEXT,
        const VkDebugUtilsMessengerCallbackDataEXT*,
        void*);

    // ── Core handles ─────────────────────────────────────────────────────────
    VkInstance               m_instance       = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
    VkSurfaceKHR             m_surface        = VK_NULL_HANDLE;
    VkPhysicalDevice         m_physicalDevice = VK_NULL_HANDLE;
    VkDevice                 m_device         = VK_NULL_HANDLE;
    VkQueue                  m_graphicsQueue  = VK_NULL_HANDLE;
    VkQueue                  m_presentQueue   = VK_NULL_HANDLE;

    // ── Swapchain ─────────────────────────────────────────────────────────────
    VkSwapchainKHR             m_swapChain           = VK_NULL_HANDLE;
    std::vector<VkImage>       m_swapChainImages;
    std::vector<VkImageView>   m_swapChainImageViews;
    VkFormat                   m_swapChainImageFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D                 m_swapChainExtent      = {};

    // ── Render pass & framebuffers ────────────────────────────────────────────
    VkRenderPass                 m_renderPass = VK_NULL_HANDLE;  // kept across recreations
    std::vector<VkFramebuffer>   m_swapChainFramebuffers;

    // ── Commands ─────────────────────────────────────────────────────────────
    VkCommandPool                m_commandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> m_commandBuffers;  // one per frame-in-flight

    // ── Descriptor pool (ImGUI) ───────────────────────────────────────────────
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;

    // ── Synchronisation ───────────────────────────────────────────────────────
    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence>     m_inFlightFences;
    std::vector<VkFence>     m_imagesInFlight;  // indexed by swapchain image

    uint32_t m_currentFrame = 0;

    QueueFamilyIndices m_queueFamilyIndices;

    // ── Config ────────────────────────────────────────────────────────────────
    const std::vector<const char*> m_validationLayers = { "VK_LAYER_KHRONOS_validation" };
    const std::vector<const char*> m_deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

#ifdef NDEBUG
    const bool m_enableValidationLayers = false;
#else
    const bool m_enableValidationLayers = true;
#endif
};
