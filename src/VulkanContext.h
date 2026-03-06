#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "core/Instance.h"
#include "core/Device.h"
#include "rendering/SwapChain.h"
#include "rendering/Renderer.h"

#include <memory>

// ─────────────────────────────────────────────────────────────────────────────
// VulkanContext  –  orchestrateur mince
//
//  Composition :  Instance  →  Device  →  SwapChain  →  Renderer
//  Expose la même API publique qu'avant : Application.cpp inchangé.
// ─────────────────────────────────────────────────────────────────────────────
class VulkanContext {
public:
    VulkanContext(GLFWwindow* window, int width, int height);
    ~VulkanContext();

    VulkanContext(const VulkanContext&)            = delete;
    VulkanContext& operator=(const VulkanContext&) = delete;

    // ── Getters (API inchangée pour Application / ImGUI) ──────────────────────
    VkInstance       getInstance()            const { return m_instance->get(); }
    VkDevice         getDevice()              const { return m_device->getLogicalDevice(); }
    VkPhysicalDevice getPhysicalDevice()      const { return m_device->getPhysicalDevice(); }
    VkRenderPass     getRenderPass()          const { return m_swapChain->getRenderPass(); }
    VkCommandPool    getCommandPool()         const { return m_renderer->getCommandPool(); }
    VkQueue          getGraphicsQueue()       const { return m_device->getGraphicsQueue(); }
    VkDescriptorPool getDescriptorPool()      const { return m_renderer->getDescriptorPool(); }
    uint32_t         getImageCount()          const { return m_swapChain->getImageCount(); }
    VkExtent2D       getSwapChainExtent()     const { return m_swapChain->getExtent(); }
    uint32_t         getGraphicsQueueFamily() const { return m_device->getQueueFamilies().graphicsFamily.value(); }
    uint32_t         getCurrentFrame()        const { return m_renderer->getCurrentFrame(); }

    // ── Frame operations ──────────────────────────────────────────────────────
    bool            beginFrame(uint32_t& outImageIndex);
    VkCommandBuffer beginCommandBuffer();
    void            beginRenderPass(VkCommandBuffer cmd, uint32_t imageIndex);
    void            endRenderPass  (VkCommandBuffer cmd);
    void            endCommandBuffer(VkCommandBuffer cmd);
    bool            submitAndPresent(VkCommandBuffer cmd, uint32_t imageIndex);

    // ── Swapchain recreation ──────────────────────────────────────────────────
    void recreateSwapChain(int width, int height);

    // ── Utility ──────────────────────────────────────────────────────────────
    VkCommandBuffer beginSingleTimeCommands();
    void            endSingleTimeCommands(VkCommandBuffer cmd);
    void            waitIdle() const;

private:
    std::unique_ptr<Instance>  m_instance;
    std::unique_ptr<Device>    m_device;
    std::unique_ptr<SwapChain> m_swapChain;
    std::unique_ptr<Renderer>  m_renderer;
};
