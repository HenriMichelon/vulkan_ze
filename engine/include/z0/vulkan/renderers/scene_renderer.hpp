#pragma once

#include "z0/vulkan/renderers/shadowmap_renderer.hpp"
#include "z0/vulkan/renderers/depth_prepass_renderer.hpp"
#include "z0/vulkan/renderers/skybox_renderer.hpp"
#include "z0/vulkan/framebuffers/color_attachment.hpp"
#include "z0/vulkan/framebuffers/color_attachment_hdr.hpp"
#include "z0/nodes/camera.hpp"
#include "z0/nodes/directional_light.hpp"
#include "z0/nodes/environment.hpp"
#include "z0/nodes/omni_light.hpp"

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
        struct GobalUniform {
            glm::mat4 projection{1.0f};
            glm::mat4 view{1.0f};
            glm::vec4 ambient = { 1.0f, 1.0f, 1.0f, .0f }; // RGB + Intensity;
            alignas(16) glm::vec3 cameraPosition;
            alignas(16) DirectionalLightUniform directionalLight;
            alignas(4) bool haveDirectionalLight{false};
            alignas(4) uint32_t pointLightsCount{0};
            alignas(4) uint32_t shadowMapsCount{0};
        };
        struct ModelUniform {
            glm::mat4 matrix;
        };
        struct MaterialUniform {
            alignas(4) int transparency;
            alignas(4) float alphaScissor;
            alignas(4) int32_t diffuseIndex{-1};
            alignas(4) int32_t specularIndex{-1};
            alignas(4) int32_t normalIndex{-1};
            alignas(16) glm::vec4 albedoColor;
            alignas(4) float shininess{32.0f};
        };

        SceneRenderer(VulkanDevice& device, std::string shaderDirectory);

        std::shared_ptr<ColorAttachmentHDR>& getColorAttachment() { return colorAttachmentHdr; }
        VkImage getImage() const override { return colorAttachmentHdr->getImage(); }
        VkImageView getImageView() const override { return colorAttachmentHdr->getImageView(); }
        std::shared_ptr<DepthBuffer>& getResolvedDepthBuffer()  { return resolvedDepthBuffer; }

        void cleanup() override;

        void setCamera(std::shared_ptr<Camera> camera);
        void setEnvironment(std::shared_ptr<Environment> environment);
        void addMesh(std::shared_ptr<MeshInstance> meshInstance);
        void addLight(std::shared_ptr<OmniLight> omniLight);

    private:
        std::shared_ptr<DirectionalLight> directionalLight{nullptr};
        std::shared_ptr<Environment> environement{nullptr};

        std::map<Node::id_t, uint32_t> modelIndices {};
        std::vector<std::shared_ptr<MeshInstance>> opaquesMeshes {};
        std::vector<std::shared_ptr<MeshInstance>> transparentsMeshes {};

        std::vector<std::shared_ptr<OmniLight>> omniLights;
        std::vector<std::unique_ptr<VulkanBuffer>> pointLightBuffers{MAX_FRAMES_IN_FLIGHT};

        std::map<Resource::rid_t, int32_t> imagesIndices {};
        std::unordered_set<std::shared_ptr<VulkanImage>> images {};

        std::vector<std::shared_ptr<Material>> materials;
        std::map<Resource::rid_t, uint32_t> materialsIndices {};
        std::vector<std::unique_ptr<VulkanBuffer>> materialsBuffers{MAX_FRAMES_IN_FLIGHT};

        // Offscreen frame buffers
        ColorAttachment colorAttachmentMultisampled;
        std::shared_ptr<ColorAttachmentHDR> colorAttachmentHdr;
        // Depth prepass buffer
        //std::shared_ptr<DepthPrepassRenderer> depthPrepassRenderer;
        std::shared_ptr<DepthBuffer> resolvedDepthBuffer;
        // Shadow mapping
        std::vector<std::shared_ptr<ShadowMap>> shadowMaps;
        //std::vector<std::shared_ptr<ShadowMapRenderer>> shadowMapRenderers;
        std::vector<std::unique_ptr<VulkanBuffer>> shadowMapsBuffers{MAX_FRAMES_IN_FLIGHT};
        // Skybox
        std::unique_ptr<SkyboxRenderer> skyboxRenderer {nullptr};

        void update(uint32_t currentFrame) override;
        void recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;
        void createDescriptorSetLayout() override;
        void loadShaders() override;
        void createImagesResources() override;
        void cleanupImagesResources() override;
        void recreateImagesResources() override;
        void beginRendering(VkCommandBuffer commandBuffer) override;
        void endRendering(VkCommandBuffer commandBuffer, bool isLast) override;

        void loadNode(std::shared_ptr<Node>& parent);
        void createImagesList(std::shared_ptr<Node>& node);
        void createImagesList(std::shared_ptr<Mesh>& mesh);
        void createImagesIndex(std::shared_ptr<Node>& node);
        void createImagesIndex(std::shared_ptr<Mesh>& mesh);
        void drawMeshes(VkCommandBuffer commandBuffer, uint32_t currentFrame, const std::vector<std::shared_ptr<MeshInstance>>& meshesToDraw);

    public:
        SceneRenderer(const SceneRenderer&) = delete;
        SceneRenderer &operator=(const SceneRenderer&) = delete;
        SceneRenderer(const SceneRenderer&&) = delete;
        SceneRenderer &&operator=(const SceneRenderer&&) = delete;
    };

}