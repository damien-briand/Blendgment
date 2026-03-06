#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include "core/Device.h"  // QueueFamilyIndices

// ─────────────────────────────────────────────────────────────────────────────
struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR        capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR>   presentModes;
};

// ─────────────────────────────────────────────────────────────────────────────
// SwapChain  –  swapchain + image views + render pass + framebuffers
//
//  Note: the render pass is created ONCE in the constructor and survives
//  calls to recreate().  Only swapchain-dependent objects are rebuilt.
// ─────────────────────────────────────────────────────────────────────────────
class SwapChain {
public:
    SwapChain(VkPhysicalDevice physDev, VkDevice device, VkSurfaceKHR surface,
              const QueueFamilyIndices& indices, int width, int height);
    ~SwapChain();

    SwapChain(const SwapChain&)            = delete;
    SwapChain& operator=(const SwapChain&) = delete;

    // ── Recreation ────────────────────────────────────────────────────────────
    void recreate(int width, int height);

    // ── Getters ───────────────────────────────────────────────────────────────
    VkSwapchainKHR getHandle()              const { return m_swapChain; }
    VkRenderPass   getRenderPass()          const { return m_renderPass; }
    VkExtent2D     getExtent()              const { return m_extent; }
    VkFormat       getFormat()              const { return m_format; }
    uint32_t       getImageCount()          const { return static_cast<uint32_t>(m_images.size()); }
    VkFramebuffer  getFramebuffer(uint32_t i) const { return m_framebuffers[i]; }

    // ── Utility ───────────────────────────────────────────────────────────────
    static SwapChainSupportDetails querySupport(VkPhysicalDevice physDev, VkSurfaceKHR surface);

private:
    void create(int width, int height);
    void cleanup();          // swapchain-dependent objects only (keeps render pass)
    void createImageViews();
    void createRenderPass(); // called once in constructor
    void createFramebuffers();

    VkSurfaceFormatKHR chooseFormat(const std::vector<VkSurfaceFormatKHR>& formats);
    VkPresentModeKHR   chooseMode  (const std::vector<VkPresentModeKHR>&   modes);
    VkExtent2D         chooseExtent(const VkSurfaceCapabilitiesKHR& caps, int w, int h);

    // ── Handles ───────────────────────────────────────────────────────────────
    VkPhysicalDevice   m_physDev;
    VkDevice           m_device;
    VkSurfaceKHR       m_surface;
    QueueFamilyIndices m_indices;

    VkSwapchainKHR           m_swapChain = VK_NULL_HANDLE;
    std::vector<VkImage>     m_images;
    std::vector<VkImageView> m_imageViews;
    VkFormat                 m_format    = VK_FORMAT_UNDEFINED;
    VkExtent2D               m_extent    = {};

    VkRenderPass               m_renderPass  = VK_NULL_HANDLE; // survives recreations
    std::vector<VkFramebuffer> m_framebuffers;
};
