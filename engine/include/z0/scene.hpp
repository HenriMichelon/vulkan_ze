#pragma once

#include <utility>

#include "z0/material.hpp"

namespace z0 {

    class Scene: public Object {
    public:
        Scene() = default;
        explicit Scene(std::shared_ptr<Node> rootNode): root{std::move(rootNode)} {};

        void setRootNode(std::shared_ptr<Node>& rootNode) { root = rootNode; }
        std::shared_ptr<Node> getRootNode() const { return root; }
        bool isValid() { return root != nullptr; }

    private:
        std::shared_ptr<Node> root;
    };

}