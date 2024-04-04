#pragma once

#include "z0/object.hpp"
#include "z0/transform.hpp"

namespace z0 {

    class Resource: public Object {
    public:
        using rid_t = unsigned int;
        Resource(std::string resName): name{resName}, id{currentId++}  { }

        rid_t getId() const { return id; }
        const std::string& getName() const { return name; }
        virtual bool isValid() = 0;
        bool operator==(const Resource& other) const { return id == other.id;}
        std::string toString() const override { return name.empty() ? Object::toString() : name; };

    protected:
        std::string name;

    private:
        rid_t id;
        static rid_t currentId;
    };

}