#include "SwapChain.h"

#include <algorithm>
#include <limits>
#include <stdexcept>

// ─────────────────────────────────────────────────────────────────────────────
SwapChain::SwapChain(VkPhysicalDevice physDev, VkDevice device, VkSurfaceKHR surface,
                     const QueueFamilyIndices& indices, int width, int height)
    : m_physDev(physDev), m_device(device), m_surface(surface), m_indices(indices)
{
    create(width, height);
    createImageViews();
    createRenderPass();    // called once here
    createFramebuffers();
}

SwapChain::~SwapChain()
{
    cleanup();
    vkDestroyRenderPass(m_device, m_renderPass, nullptr);
}

// ─────────────────────────────────────────────────────────────────────────────
void SwapChain::recreate(int width, int height)
{
    vkDeviceWaitIdle(m_device);
    cleanup();
    create(width, height);
    createImageViews();
    createFramebuffers();  // render pass is reused
}

// ─────────────────────────────────────────────────────────────────────────────
void SwapChain::create(int width, int height)
{
    auto support      = querySupport(m_physDev, m_surface);
    auto surfaceFormat= chooseFormat(support.formats);
    auto presentMode  = chooseMode  (support.presentModes);
    auto extent       = chooseExtent(support.capabilities, width, height);

    uint32_t imageCount = support.capabilities.minImageCount + 1;
    if (support.capabilities.maxImageCount > 0 && imageCount > support.capabilities.maxImageCount)
        imageCount = support.capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR ci{};
    ci.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    ci.surface          = m_surface;
    ci.minImageCount    = imageCount;
    ci.imageFormat      = surfaceFormat.format;
    ci.imageColorSpace  = surfaceFormat.colorSpace;
    ci.imageExtent      = extent;
    ci.imageArrayLayers = 1;
    ci.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t families[] = { m_indices.graphicsFamily.value(), m_indices.presentFamily.value() };
    if (families[0] != families[1]) {
        ci.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        ci.queueFamilyIndexCount = 2;
        ci.pQueueFamilyIndices   = families;
    } else {
        ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    ci.preTransform   = support.capabilities.currentTransform;
    ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    ci.presentMode    = presentMode;
    ci.clipped        = VK_TRUE;
    ci.oldSwapchain   = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(m_device, &ci, nullptr, &m_swapChain) != VK_SUCCESS)
        throw std::runtime_error("Failed to create swapchain");

    vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr);
    m_images.resize(imageCount);
    vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, m_images.data());

    m_format = surfaceFormat.format;
    m_extent = extent;
}

void SwapChain::cleanup()
{
    for (auto fb : m_framebuffers)    vkDestroyFramebuffer(m_device, fb, nullptr);
    m_framebuffers.clear();

    for (auto iv : m_imageViews)      vkDestroyImageView(m_device, iv, nullptr);
    m_imageViews.clear();

    if (m_swapChain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
        m_swapChain = VK_NULL_HANDLE;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void SwapChain::createImageViews()
{
    m_imageViews.resize(m_images.size());
    for (size_t i = 0; i < m_images.size(); ++i) {
        VkImageViewCreateInfo ci{};
        ci.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ci.image                           = m_images[i];
        ci.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
        ci.format                          = m_format;
        ci.components                      = { VK_COMPONENT_SWIZZLE_IDENTITY,
                                               VK_COMPONENT_SWIZZLE_IDENTITY,
                                               VK_COMPONENT_SWIZZLE_IDENTITY,
                                               VK_COMPONENT_SWIZZLE_IDENTITY };
        ci.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        ci.subresourceRange.baseMipLevel   = 0;
        ci.subresourceRange.levelCount     = 1;
        ci.subresourceRange.baseArrayLayer = 0;
        ci.subresourceRange.layerCount     = 1;

        if (vkCreateImageView(m_device, &ci, nullptr, &m_imageViews[i]) != VK_SUCCESS)
            throw std::runtime_error("Failed to create image view");
    }
}

void SwapChain::createRenderPass()
{
    VkAttachmentDescription color{};
    color.format         = m_format;
    color.samples        = VK_SAMPLE_COUNT_1_BIT;
    color.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    color.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    color.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorRef{};
    colorRef.attachment = 0;
    colorRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments    = &colorRef;

    VkSubpassDependency dep{};
    dep.srcSubpass    = VK_SUBPASS_EXTERNAL;
    dep.dstSubpass    = 0;
    dep.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dep.srcAccessMask = 0;
    dep.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo rpInfo{};
    rpInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rpInfo.attachmentCount = 1;
    rpInfo.pAttachments    = &color;
    rpInfo.subpassCount    = 1;
    rpInfo.pSubpasses      = &subpass;
    rpInfo.dependencyCount = 1;
    rpInfo.pDependencies   = &dep;

    if (vkCreateRenderPass(m_device, &rpInfo, nullptr, &m_renderPass) != VK_SUCCESS)
        throw std::runtime_error("Failed to create render pass");
}

void SwapChain::createFramebuffers()
{
    m_framebuffers.resize(m_imageViews.size());
    for (size_t i = 0; i < m_imageViews.size(); ++i) {
        VkImageView attachments[] = { m_imageViews[i] };

        VkFramebufferCreateInfo fi{};
        fi.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fi.renderPass      = m_renderPass;
        fi.attachmentCount = 1;
        fi.pAttachments    = attachments;
        fi.width           = m_extent.width;
        fi.height          = m_extent.height;
        fi.layers          = 1;

        if (vkCreateFramebuffer(m_device, &fi, nullptr, &m_framebuffers[i]) != VK_SUCCESS)
            throw std::runtime_error("Failed to create framebuffer");
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// static
SwapChainSupportDetails SwapChain::querySupport(VkPhysicalDevice physDev, VkSurfaceKHR surface)
{
    SwapChainSupportDetails d;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physDev, surface, &d.capabilities);

    uint32_t count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physDev, surface, &count, nullptr);
    if (count) {
        d.formats.resize(count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physDev, surface, &count, d.formats.data());
    }
    vkGetPhysicalDeviceSurfacePresentModesKHR(physDev, surface, &count, nullptr);
    if (count) {
        d.presentModes.resize(count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physDev, surface, &count, d.presentModes.data());
    }
    return d;
}

VkSurfaceFormatKHR SwapChain::chooseFormat(const std::vector<VkSurfaceFormatKHR>& formats)
{
    for (const auto& f : formats)
        if (f.format == VK_FORMAT_B8G8R8A8_UNORM && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return f;
    return formats[0];
}

VkPresentModeKHR SwapChain::chooseMode(const std::vector<VkPresentModeKHR>& modes)
{
    for (const auto& m : modes)
        if (m == VK_PRESENT_MODE_MAILBOX_KHR) return m;
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapChain::chooseExtent(const VkSurfaceCapabilitiesKHR& caps, int w, int h)
{
    if (caps.currentExtent.width != std::numeric_limits<uint32_t>::max())
        return caps.currentExtent;

    return {
        std::clamp(static_cast<uint32_t>(w), caps.minImageExtent.width,  caps.maxImageExtent.width),
        std::clamp(static_cast<uint32_t>(h), caps.minImageExtent.height, caps.maxImageExtent.height)
    };
}
