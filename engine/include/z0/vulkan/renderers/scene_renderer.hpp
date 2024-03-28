#pragma once

#include "z0/vulkan/renderers/shadowmap_renderer.hpp"
#include "z0/vulkan/renderers/depth_prepass_renderer.hpp"
#include "z0/vulkan/renderers/skybox_renderer.hpp"
#include "z0/vulkan/framebuffers/tone_map.hpp"
#include "z0/vulkan/framebuffers/multisampled.hpp"
#include "z0/nodes/camera.hpp"
#include "z0/nodes/directional_light.hpp"
#include "z0/nodes/environment.hpp"
#include "z0/nodes/omni_light.hpp"
#include "tonemapping_renderer.hpp"

#include <map>

namespace z0 {

    class SceneRenderer: public BaseMeshesRenderer {
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
        struct ShadowMapUniform {
            glm::mat4 lightSpace;
            alignas(16) glm::vec3 lightPos;
        };
        struct GobalUniformBufferObject {
            glm::mat4 projection{1.0f};
            glm::mat4 view{1.0f};
            glm::vec4 ambient = { 1.0f, 1.0f, 1.0f, .0f }; // RGB + Intensity;
            alignas(16) glm::vec3 cameraPosition;
            alignas(16) DirectionalLightUniform directionalLight;
            alignas(4) bool haveDirectionalLight{false};
            alignas(4) uint32_t pointLightsCount{0};
            alignas(4) uint32_t shadowMapsCount{0};
        };
        struct ModelUniformBufferObject {
            glm::mat4 matrix;
        };
        struct SurfaceUniformBufferObject {
            alignas(4) int transparency;
            alignas(4) float alphaScissor;
            alignas(4) int32_t diffuseIndex{-1};
            alignas(4) int32_t specularIndex{-1};
            alignas(4) int32_t normalIndex{-1};
            alignas(16) glm::vec4 albedoColor;
            alignas(4) float shininess{32.0f};
        };

        SceneRenderer(VulkanDevice& device, std::string shaderDirectory);

        void loadScene(std::shared_ptr<Node>& rootNode);
        void cleanup() override;

        const std::shared_ptr<TonemappingRenderer>& getTonemappingRenderer() const { return tonemappingRenderer; }

    private:
        DirectionalLight* directionalLight{nullptr};
        Environment* environement{nullptr};

        std::map<Node::id_t, uint32_t> modelIndices {};
        std::vector<MeshInstance*> opaquesMeshes {};
        std::vector<MeshInstance*> transparentsMeshes {};
        std::vector<OmniLight*> omniLights;
        std::vector<std::unique_ptr<VulkanBuffer>> pointLightBuffers{MAX_FRAMES_IN_FLIGHT};
        std::map<Resource::rid_t, int32_t> imagesIndices {};
        std::unordered_set<std::shared_ptr<VulkanImage>> images {};
        std::map<Resource::rid_t, uint32_t> surfacesIndices {};
        std::vector<std::unique_ptr<VulkanBuffer>> surfacesBuffers{MAX_FRAMES_IN_FLIGHT};

        // Multisampled offscreen frame buffer
        Multisampled multisampled;
        // Non multisampled offscreen frame buffer
        ToneMap toneMap;
        // Blit last offscreen frame buffer to swapchain
        VkImageBlit colorImageBlit{};
        // Depth prepass buffer
        std::shared_ptr<DepthPrepassRenderer> depthPrepassRenderer;
        // Shadow mapping
        std::vector<std::shared_ptr<ShadowMap>> shadowMaps;
        std::vector<std::shared_ptr<ShadowMapRenderer>> shadowMapRenderers;
        std::vector<std::unique_ptr<VulkanBuffer>> shadowMapsBuffers{MAX_FRAMES_IN_FLIGHT};
        // Skybox
        std::unique_ptr<SkyboxRenderer> skyboxRenderer {nullptr};
        // HDR -> LDR
        std::shared_ptr<TonemappingRenderer> tonemappingRenderer;

        void update(uint32_t currentFrame) override;
        void recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;
        void createDescriptorSetLayout() override;
        void loadShaders() override;
        void createImagesResources() override;
        void cleanupImagesResources() override;
        void recreateImagesResources() override;
        void beginRendering(VkCommandBuffer commandBuffer) override;
        void endRendering(VkCommandBuffer commandBuffer, VkImage swapChainImage) override;

        void loadNode(std::shared_ptr<Node>& parent);
        void createImagesList(std::shared_ptr<Node>& node);
        void createImagesIndex(std::shared_ptr<Node>& node);
        void drawMeshes(VkCommandBuffer commandBuffer, uint32_t currentFrame, const std::vector<MeshInstance*>& meshesToDraw);

    public:
        SceneRenderer(const SceneRenderer&) = delete;
        SceneRenderer &operator=(const SceneRenderer&) = delete;
        SceneRenderer(const SceneRenderer&&) = delete;
        SceneRenderer &&operator=(const SceneRenderer&&) = delete;
    };

}