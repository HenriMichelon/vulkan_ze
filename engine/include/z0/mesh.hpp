#pragma once

#include "z0/viewport.hpp"

#include "z0/vulkan/vulkan_model.hpp"
#include "texture.hpp"

namespace z0 {

    class Mesh: public Resource {
    public:
        Mesh(Viewport &viewport, const std::string& appdir, const std::string& filename, std::shared_ptr<Texture> texture);

        Texture& getTexture() { return *texture; }

    private:
        Viewport& viewport;
        std::shared_ptr<Texture> texture;
        std::shared_ptr<VulkanModel> vulkanModel;

    public:
        VulkanModel& _getModel() { return *vulkanModel; }
    };

}