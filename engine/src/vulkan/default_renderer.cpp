#include "z0/vulkan/default_renderer.hpp"
#include "z0/log.hpp"
#include "z0/nodes/camera.hpp"

namespace z0 {

    DefaultRenderer::DefaultRenderer(VulkanDevice &dev,
                                     const std::string& sDir) :
         VulkanRenderer{dev, sDir}
     {
     }

    DefaultRenderer::~DefaultRenderer() {
        vkDeviceWaitIdle(device);
    }

    void DefaultRenderer::loadScene(const std::shared_ptr<Node>& root) {
        rootNode = root;
        createMeshIndices(rootNode);
        loadResources();
    }

    void DefaultRenderer::createMeshIndices(const std::shared_ptr<Node>& parent) {
        createMeshIndex(parent);
        for(const auto& node: parent->getChildren()) {
            createMeshIndices(node);
        }
    }

    void DefaultRenderer::createMeshIndex(const std::shared_ptr<Node>& node) {
        if (auto meshInstance = dynamic_cast<MeshInstance*>(node.get())) {
            meshInstances.push_back(meshInstance);
            auto it = meshes.insert(meshInstance->getMesh());
            meshesIndices[meshInstance] = std::distance(std::begin(meshes), it.first);
        }
    }

    void DefaultRenderer::loadShaders() {
        vertShader = createShader("default.vert", VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT);
        fragShader = createShader("default.frag", VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    }

    void DefaultRenderer::update(float delta) {
        Camera camera{};
        camera.transform.position = { -0.0f, 0.0f, -5.0f };
        camera.setViewYXZ();
        camera.setPerspectiveProjection(glm::radians(50.0f), getAspectRatio(), 0.1f, 100.0f);

        UniformBufferObject ubo{
            .projection = camera.getProjection(),
            .view = camera.getView(),
            .inverseView = camera.getInverseView(),
        };

        for (int index = 0; index < meshInstances.size(); index++) {
            auto meshInstance = meshInstances[index];
            ubo.model = meshInstance->transform.mat4();
            ubo.textureIndex = meshesIndices[meshInstance];
            writeUniformBuffer(&ubo, index);
        }
    }

    void DefaultRenderer::recordCommands(VkCommandBuffer commandBuffer) {
        if (meshInstances.empty()) return;
        vkCmdSetCullMode(commandBuffer, VK_CULL_MODE_NONE);
        vkCmdSetDepthWriteEnable(commandBuffer, VK_TRUE);
        bindShader(commandBuffer, *vertShader);
        bindShader(commandBuffer, *fragShader);
        for (int index = 0; index < meshInstances.size(); index++) {
            if (meshInstances[index]->getMesh()->isValid()) {
                bindDescriptorSets(commandBuffer, index);
                meshInstances[index]->getMesh()->_getModel().draw(commandBuffer);
            }
        }
    }

    void DefaultRenderer::createDescriptorSetLayout() {
        if (meshInstances.empty()) return;
        VkDeviceSize size = sizeof(UniformBufferObject);
        createUniformBuffers(size, meshInstances.size());
        globalSetLayout = VulkanDescriptorSetLayout::Builder(vulkanDevice)
                .addBinding(0,
                            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                            VK_SHADER_STAGE_ALL_GRAPHICS)
                .addBinding(1,
                            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                            VK_SHADER_STAGE_ALL_GRAPHICS,
                            meshes.size())
                .build();
        for (int i = 0; i < globalDescriptorSets.size(); i++) {
            auto bufferInfo = uboBuffers[i]->descriptorInfo(size);
            std::vector<VkDescriptorImageInfo> imagesInfo{};
            for(const auto& mesh: meshes) {
                imagesInfo.push_back(mesh->getSurfaceMaterial(0)->albedo_texture->getImage()._getImage().imageInfo());
            }
            if (!VulkanDescriptorWriter(*globalSetLayout, *globalPool)
                    .writeBuffer(0, &bufferInfo)
                    .writeImage(1, imagesInfo.data())
                    .build(globalDescriptorSets[i])) {
                die("Cannot allocate descriptor set");
            }
        }
    }

}