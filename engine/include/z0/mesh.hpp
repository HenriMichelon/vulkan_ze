#pragma once

#include "z0/viewport.hpp"

#include "z0/vulkan/vulkan_model.hpp"

namespace z0 {

    class Mesh: public Object {
    public:
        Mesh(Viewport &viewport, const std::string& filename);

    private:
        Viewport &viewport;
        std::shared_ptr<VulkanModel> vulkanModel;

    public:
        VulkanModel& _getModel() { return *vulkanModel; }
    };

}