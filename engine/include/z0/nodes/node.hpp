#pragma once

#include "z0/transform.hpp"
#include "z0/object.hpp"

namespace z0 {

    class Node: public Object {
    public:
        using id_t = unsigned int;
        using Map = std::unordered_map<id_t, Node>;
        Transform transform{};

        Node(): id{currentId++}  { }

        id_t getId() const { return id; }

        virtual void onReady() {};
        virtual void onProcess(float delta) {};

        void addChild(const std::shared_ptr<Node>& node);
        void removeChild(const std::shared_ptr<Node>& node);
        std::list<std::shared_ptr<Node>>& getChildren() { return children; }

        bool operator==(const Node& other) const { return id == other.id;}

    private:
        id_t id;
        static id_t currentId;
        std::list<std::shared_ptr<Node>> children;
    };
}