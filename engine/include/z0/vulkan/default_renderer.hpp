#pragma once

#include "z0/vulkan/vulkan_renderer.hpp"
#include "z0/nodes/mesh_instance.hpp"
#include "z0/nodes/camera.hpp"
#include "z0/nodes/directional_light.hpp"
#include "z0/nodes/environment.hpp"
#include "z0/nodes/omni_light.hpp"
#include "z0/nodes/spot_light.hpp"

namespace z0 {

    class DefaultRenderer: public VulkanRenderer {
    public:
        struct DirectionalLightUniform {
            alignas(16) glm::vec3 direction = { 0.0f, 0.0f, 0.0f };
            alignas(16) glm::vec4 color = { 0.0f, 0.0f, 0.0f, 0.0f }; // RGB + Intensity;
            alignas(4) float specular = { 1.0f };
        };
        struct PointLightUniform {
            alignas(16) glm::vec3 position = {0.0f, 0.0f, 0.0f};
            alignas(16) glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f }; // RGB + Intensity;
            alignas(4) float specular = { 1.0f };
            alignas(4) float constant = { 1.0f };
            alignas(4) float linear{0.0};
            alignas(4) float quadratic{0.0};
        };
        struct SpotLightUniform {
            alignas(16) glm::vec3 position = { 0.0f, 0.0f, 0.0f };
            alignas(16) glm::vec3 direction = glm::normalize(glm::vec3{0.f, .0f, 1.0f});
            alignas(16) glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f }; // RGB + Intensity;
            alignas(4) float cutOff = { glm::cos(glm::radians(10.f)) };
            alignas(4) float outerCutOff = { glm::cos(glm::radians(15.f)) };
            alignas(4) float specular = { 1.0f };
            alignas(4) float constant = { 1.0f };
            alignas(4) float linear{0.0};
            alignas(4) float quadratic{0.00};
        };
        struct GobalUniformBufferObject {
            glm::mat4 projection{1.0f};
            glm::mat4 view{1.0f};
            glm::vec4 ambient = { 1.0f, 1.0f, 1.0f, .0f }; // RGB + Intensity;
            alignas(16) glm::vec3 cameraPosition;
            alignas(16) DirectionalLightUniform directionalLight;
            alignas(4) bool haveDirectionalLight{false};
            alignas(4) uint32_t pointLightsCount{0};
            alignas(4) uint32_t spotLightsCount{0};
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

        DefaultRenderer(VulkanDevice& device, const std::string& shaderDirectory);
        ~DefaultRenderer();

        void loadScene(const std::shared_ptr<Node>& rootNode);

    private:
        Camera* currentCamera{nullptr};
        DirectionalLight* directionalLight;
        Environment* environement;
        std::vector<OmniLight*> omniLights;
        std::vector<SpotLight*> spotLights;

        std::unique_ptr<VulkanShader> vertShader;
        std::unique_ptr<VulkanShader> fragShader;
        std::shared_ptr<Node> rootNode;
        std::vector<MeshInstance*> meshes {};
        std::unordered_set<std::shared_ptr<Texture>> textures {};
        std::map<Resource::rid_t, int32_t> texturesIndices {};

        std::unique_ptr<VulkanDescriptorPool> globalPool {};
        std::vector<std::unique_ptr<VulkanBuffer>> globalBuffers{MAX_FRAMES_IN_FLIGHT};
        std::vector<std::unique_ptr<VulkanBuffer>> modelsBuffers{MAX_FRAMES_IN_FLIGHT};
        std::vector<std::unique_ptr<VulkanBuffer>> surfacesBuffers{MAX_FRAMES_IN_FLIGHT};
        std::vector<std::unique_ptr<VulkanBuffer>> pointLightBuffers{MAX_FRAMES_IN_FLIGHT};
        std::vector<std::unique_ptr<VulkanBuffer>> spotLightBuffers{MAX_FRAMES_IN_FLIGHT};

        void update(float delta) override;
        void recordCommands(VkCommandBuffer commandBuffer) override;
        void createDescriptorSetLayout() override;
        void loadShaders() override;

        void loadNode(std::shared_ptr<Node>& parent);
        void createMeshIndex(std::shared_ptr<Node>& node);

    public:
        DefaultRenderer(const DefaultRenderer&) = delete;
        DefaultRenderer &operator=(const DefaultRenderer&) = delete;
        DefaultRenderer(const DefaultRenderer&&) = delete;
        DefaultRenderer &&operator=(const DefaultRenderer&&) = delete;
    };

}