#include "z0/nodes/node.hpp"

namespace z0 {

    Node::id_t Node::currentId = 0;

    void Node::addChild(const z0::Node& node) {
        children.push_back(node);
    }

    void Node::removeChild(const z0::Node& node) {
        children.remove(node);
    }


}