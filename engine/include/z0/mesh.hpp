#pragma once

#include "object.hpp"
#include "viewport.hpp"

namespace z0 {

    class Mesh: public Object {
    public:
        Mesh(Viewport &viewport, const std::string& filename);

    private:
        Viewport &viewport;
        std::unique_ptr<VulkanModel> vulkanModel;
    };

}