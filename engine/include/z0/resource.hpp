#pragma once

#include "z0/object.hpp"
#include "z0/transform.hpp"

namespace z0 {

    class Resource: public Object {
    public:
        using rid_t = unsigned int;
        Transform transform{};

        Resource(): id{currentId++} { }

        rid_t getId() const { return id; }

        bool operator==(const Resource& other) const {
            return id == other.id;
        }

    private:
        rid_t id;
        static rid_t currentId;

    };
}