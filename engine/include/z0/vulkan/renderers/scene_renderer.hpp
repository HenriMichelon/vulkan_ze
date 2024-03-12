#pragma once

#include "z0/vulkan/vulkan_renderer.hpp"
#include "z0/vulkan/renderers/shadowmap_renderer.hpp"
#include "z0/nodes/mesh_instance.hpp"
#include "z0/nodes/camera.hpp"
#include "z0/nodes/directional_light.hpp"
#include "z0/nodes/environment.hpp"
#include "z0/nodes/omni_light.hpp"
#include "z0/nodes/spot_light.hpp"

namespace z0 {

    class SceneRenderer: public VulkanRenderer {
    public:
        struct DirectionalLightUniform {
            alignas(16) glm::vec3 direction = { 0.0f, 0.0f, 0.0f };
            alignas(16) glm::vec4 color = { 0.0f, 0.0f, 0.0f, 0.0f }; // RGB + Intensity;
            alignas(4) float specular = { 1.0f };
        };
        struct PointLightUniform {
            alignas(16) glm::vec3 position = { 0.0f, 0.0f, 0.0f };
            alignas(16) glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f }; // RGB + Intensity;
            alignas(4) float specular = { 1.0f };
            alignas(4) float constant = { 1.0f };
            alignas(4) float linear{0.0};
            alignas(4) float quadratic{0.00};
            alignas(4) bool isSpot{false};
            alignas(16) glm::vec3 direction = glm::normalize(glm::vec3{0.f, .0f, .0f});
            alignas(4) float cutOff = { glm::cos(glm::radians(10.f)) };
            alignas(4) float outerCutOff = { glm::cos(glm::radians(15.f)) };
        };
        struct GobalUniformBufferObject {
            glm::mat4 projection{1.0f};
            glm::mat4 view{1.0f};
            glm::vec4 ambient = { 1.0f, 1.0f, 1.0f, .0f }; // RGB + Intensity;
            alignas(16) glm::vec3 cameraPosition;
            alignas(16) DirectionalLightUniform directionalLight;
            alignas(4) bool haveDirectionalLight{false};
            alignas(4) uint32_t pointLightsCount{0};
        };
        struct ModelUniformBufferObject {
            glm::mat4 matrix;
            glm::mat4 normalMatrix;
        };
        struct SurfaceUniformBufferObject {
            alignas(4) int32_t diffuseIndex{-1};
            alignas(4) int32_t specularIndex{-1};
            alignas(16) glm::vec4 albedoColor;
            alignas(4) float shininess{32.0f};
        };

        SceneRenderer(VulkanDevice& device, const std::string& shaderDirectory);
        ~SceneRenderer();

        void drawFrame() override;
        void loadScene(std::shared_ptr<Node>& rootNode);

    private:
        std::vector<MeshInstance*> meshes {};
        Camera* currentCamera{nullptr};
        DirectionalLight* directionalLight{nullptr};
        Environment* environement{nullptr};
        std::vector<OmniLight*> omniLights;

        ShadowMapRenderer shadowMapRenderer;
        std::vector<std::shared_ptr<ShadowMap>> shadowMaps;

        std::unique_ptr<VulkanShader> vertShader;
        std::unique_ptr<VulkanShader> fragShader;
        std::unordered_set<std::shared_ptr<VulkanImage>> images {};
        std::map<Resource::rid_t, int32_t> imagesIndices {};

        std::unique_ptr<VulkanDescriptorPool> globalPool {};
        std::vector<std::unique_ptr<VulkanBuffer>> globalBuffers{MAX_FRAMES_IN_FLIGHT};
        std::vector<std::unique_ptr<VulkanBuffer>> modelsBuffers{MAX_FRAMES_IN_FLIGHT};
        std::vector<std::unique_ptr<VulkanBuffer>> surfacesBuffers{MAX_FRAMES_IN_FLIGHT};
        std::vector<std::unique_ptr<VulkanBuffer>> pointLightBuffers{MAX_FRAMES_IN_FLIGHT};

        // Depth buffering
        VkImage depthImage;
        VkFormat depthFormat;
        VkDeviceMemory depthImageMemory;
        VkImageView depthImageView;

        // Offscreen frame buffer for MSAA
        VkImage colorImage;
        VkDeviceMemory colorImageMemory;
        VkImageView colorImageView;
        VkImageBlit colorImageBlit{};
        VkImageResolve colorImageResolve{};

        void update() override;
        void recordCommands(VkCommandBuffer commandBuffer) override;
        void createDescriptorSetLayout() override;
        void loadShaders() override;
        void createImagesResources() override;
        void cleanupImagesResources() override;
        void beginRendering(VkCommandBuffer commandBuffer, uint32_t imageIndex) override;
        void endRendering(VkCommandBuffer commandBuffer,uint32_t imageIndex) override;

        void loadNode(std::shared_ptr<Node>& parent);
        void createImagesList(std::shared_ptr<Node>& node);
        void createImagesIndex(std::shared_ptr<Node>& node);

    public:
        SceneRenderer(const SceneRenderer&) = delete;
        SceneRenderer &operator=(const SceneRenderer&) = delete;
        SceneRenderer(const SceneRenderer&&) = delete;
        SceneRenderer &&operator=(const SceneRenderer&&) = delete;
    };

}