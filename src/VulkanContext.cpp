#include "VulkanContext.h"
#include <iostream>

// ─────────────────────────────────────────────────────────────────────────────
VulkanContext::VulkanContext(GLFWwindow *window, int width, int height)
{
    m_instance = std::make_unique<Instance>(window);
    m_device = std::make_unique<Device>(m_instance->get(), m_instance->getSurface());
    m_swapChain = std::make_unique<SwapChain>(
        m_device->getPhysicalDevice(),
        m_device->getLogicalDevice(),
        m_instance->getSurface(),
        m_device->getQueueFamilies(),
        width, height);
    m_renderer = std::make_unique<Renderer>(
        m_device->getLogicalDevice(),
        m_device->getGraphicsQueue(),
        m_device->getPresentQueue(),
        m_device->getQueueFamilies().graphicsFamily.value(),
        m_swapChain->getImageCount());

    std::cout << "[VulkanContext] Initialized.\n";
}

VulkanContext::~VulkanContext()
{
    waitIdle();
    // Ordre de destruction : Renderer → SwapChain → Device → Instance
    m_renderer.reset();
    m_swapChain.reset();
    m_device.reset();
    m_instance.reset();
}

// ─────────────────────────────────────────────────────────────────────────────
// Délégation vers Renderer / SwapChain
// ─────────────────────────────────────────────────────────────────────────────
bool VulkanContext::beginFrame(uint32_t &outImageIndex)
{
    return m_renderer->beginFrame(m_swapChain->getHandle(), outImageIndex);
}

VkCommandBuffer VulkanContext::beginCommandBuffer()
{
    return m_renderer->beginCommandBuffer();
}

void VulkanContext::beginRenderPass(VkCommandBuffer cmd, uint32_t imageIndex)
{
    m_renderer->beginRenderPass(
        cmd, imageIndex,
        m_swapChain->getRenderPass(),
        m_swapChain->getFramebuffer(imageIndex),
        m_swapChain->getExtent());
}

void VulkanContext::endRenderPass(VkCommandBuffer cmd) { m_renderer->endRenderPass(cmd); }
void VulkanContext::endCommandBuffer(VkCommandBuffer cmd) { m_renderer->endCommandBuffer(cmd); }

bool VulkanContext::submitAndPresent(VkCommandBuffer cmd, uint32_t imageIndex)
{
    return m_renderer->submitAndPresent(cmd, imageIndex, m_swapChain->getHandle());
}

void VulkanContext::recreateSwapChain(int width, int height)
{
    waitIdle();
    m_swapChain->recreate(width, height);
    m_renderer->onSwapChainRecreated(m_swapChain->getImageCount());
}

VkCommandBuffer VulkanContext::beginSingleTimeCommands() { return m_renderer->beginSingleTimeCommands(); }
void VulkanContext::endSingleTimeCommands(VkCommandBuffer cmd) { m_renderer->endSingleTimeCommands(cmd); }
void VulkanContext::waitIdle() const { m_device->waitIdle(); }
