#include "z0/vulkan/framebuffers/depth_buffer.hpp"
#include "z0/log.hpp"

namespace z0 {

    DepthBuffer::DepthBuffer(VulkanDevice &dev, bool _multisampled) : BaseFrameBuffer{dev}, multisampled{_multisampled} {
         createImagesResources();
     }

    // https://vulkan-tutorial.com/Depth_buffering#page_Depth-image-and-view
    void DepthBuffer::createImagesResources() {
        createImage(vulkanDevice.getSwapChainExtent().width,
                    vulkanDevice.getSwapChainExtent().height,
                    vulkanDevice.findImageTilingSupportedFormat(
                            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D16_UNORM, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT,},
                            VK_IMAGE_TILING_OPTIMAL,
                            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT),
                    multisampled ? vulkanDevice.getSamples() : VK_SAMPLE_COUNT_1_BIT,
                    multisampled ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_IMAGE_ASPECT_DEPTH_BIT);
    }

}