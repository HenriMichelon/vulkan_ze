#pragma once

#include "z0/object.hpp"
#include "z0/transform.hpp"

namespace z0 {

    class Node: public Object {
    public:
        using id_t = unsigned int;
        using Map = std::unordered_map<id_t, Node>;
        Transform transform{};

        Node(): id{currentId++} { }

        id_t getId() const { return id; }

    private:
        id_t id;
        static id_t currentId;


    };
}