#include "z0/vulkan/renderers/tonemapping_renderer.hpp"
#include "z0/log.hpp"
#include "z0/application.hpp"

namespace z0 {

    TonemappingRenderer::TonemappingRenderer(VulkanDevice &dev,
                                             std::string shaderDirectory,
                                             std::shared_ptr<ColorAttachmentHDR>& _inputColorAttachmentHdr,
                                             std::shared_ptr<DepthBuffer> _depthBuffer):
            BasePostprocessingRenderer{dev, shaderDirectory, _inputColorAttachmentHdr},
            resolvedDepthBuffer{_depthBuffer} {
        createResources();
    }

    void TonemappingRenderer::loadShaders() {
        BasePostprocessingRenderer::loadShaders();
        fragShader = createShader("reinhard.frag", VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    }

    void TonemappingRenderer::update(uint32_t currentFrame) {
        GobalUniformBufferObject globalUbo {
            .gamma = Application::getConfig().gamma,
            .exposure = Application::getConfig().exposure,
        };
        writeUniformBuffer(globalBuffers, currentFrame, &globalUbo);
    }

    void TonemappingRenderer::recreateImagesResources() {
        colorAttachmentHdr->cleanupImagesResources();
        colorAttachmentHdr->createImagesResources();
        for (int i = 0; i < descriptorSets.size(); i++) {
            auto globalBufferInfo = globalBuffers[i]->descriptorInfo(sizeof(GobalUniformBufferObject));
            auto imageInfo = inputColorAttachmentHdr->imageInfo();
            VkDescriptorImageInfo depthImageInfo {
                    .sampler = imageInfo.sampler,
                    .imageView = resolvedDepthBuffer->getImageView(),
                    .imageLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
            };
            auto writer = VulkanDescriptorWriter(*globalSetLayout, *globalPool)
                    .writeBuffer(0, &globalBufferInfo)
                    .writeImage(1, &imageInfo)
                    .writeImage(2, &depthImageInfo);
            writer.overwrite(descriptorSets[i]);
        }
    }

    void TonemappingRenderer::createDescriptorSetLayout() {
        globalPool = VulkanDescriptorPool::Builder(vulkanDevice)
                .setMaxSets(MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMES_IN_FLIGHT)
                .build();
        createUniformBuffers(globalBuffers, sizeof(GobalUniformBufferObject));
        globalSetLayout = VulkanDescriptorSetLayout::Builder(vulkanDevice)
                .addBinding(0,
                            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                            VK_SHADER_STAGE_FRAGMENT_BIT)
                .addBinding(1,
                            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                            VK_SHADER_STAGE_FRAGMENT_BIT,
                            1)
                .addBinding(2,
                            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                            VK_SHADER_STAGE_FRAGMENT_BIT,
                            1)
                .build();
        for (int i = 0; i < descriptorSets.size(); i++) {
            auto globalBufferInfo = globalBuffers[i]->descriptorInfo(sizeof(GobalUniformBufferObject));
            auto imageInfo = inputColorAttachmentHdr->imageInfo();
            VkDescriptorImageInfo depthImageInfo {
                .sampler = imageInfo.sampler,
                .imageView = resolvedDepthBuffer->getImageView(),
                .imageLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
            };
            auto writer = VulkanDescriptorWriter(*globalSetLayout, *globalPool)
                    .writeBuffer(0, &globalBufferInfo)
                    .writeImage(1, &imageInfo)
                    .writeImage(2, &depthImageInfo);
            if (!writer.build(descriptorSets[i])) {
                die("Cannot allocate descriptor set");
            }
        }
    }


}