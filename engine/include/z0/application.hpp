#pragma once

#include "z0/helpers/window_helper.hpp"
#include "z0/application_config.hpp"
#include "z0/vulkan/vulkan_instance.hpp"
#include "z0/nodes/node.hpp"
#include "z0/utils/blocking_queue.hpp"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Core/JobSystemThreadPool.h>

namespace z0 {

    // Class that determines if two nodes can collide
    class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter {
    public:
        bool ShouldCollide(JPH::ObjectLayer layersAndMask1, JPH::ObjectLayer layersAndMask2) const override {
            auto sourceMask = layersAndMask1 & 0b1111;
            auto targetLayer = (layersAndMask2 >> 4) & 0b1111;
            return (targetLayer & sourceMask) != 0;
        }
    };

    // This defines a mapping between object and broadphase layers.
    class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface {
    public:
        uint32_t GetNumBroadPhaseLayers() const override {
            return 1;
        }

        JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override {
            return static_cast<JPH::BroadPhaseLayer>(0);
        }

        const char * GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override {
            return "?";
        }};

    // Class that determines if an object layer can collide with a broadphase layer
    class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter {
    public:
        bool ShouldCollide(JPH::ObjectLayer layers, JPH::BroadPhaseLayer masks) const override{
            return true;
        }
    };

    class Viewport;
    const float dt = 0.01; // Fixed delta time

    class Application: public Object {
    public:
        explicit Application(const ApplicationConfig& applicationConfig);

        void start(const std::shared_ptr<Node>& scene);

        static bool isPaused() { return get().paused; }
        static void setPaused(bool pause) { get().paused = pause; }
        static std::shared_ptr<Node>& getCurrentScene() { return get().currentScene; }
        static Viewport& getViewport() { return *get().viewport; }
        static const std::filesystem::path getDirectory() { return get().applicationConfig.appDir; }
        static const ApplicationConfig& getConfig() { return get().applicationConfig; }

    private:
        VulkanInstance vulkanInstance;
        std::shared_ptr<Viewport> viewport;
        const ApplicationConfig& applicationConfig;
        std::shared_ptr<Node> currentScene;
        bool paused{false};
        BlockingQueue<std::shared_ptr<Node>> queue{1000};

        JPH::PhysicsSystem physicsSystem;
        BPLayerInterfaceImpl broad_phase_layer_interface;
        ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;
        ObjectLayerPairFilterImpl object_vs_object_layer_filter;
        std::unique_ptr<JPH::TempAllocatorImpl> temp_allocator;
        std::unique_ptr<JPH::JobSystemThreadPool> job_system;

        void ready(const std::shared_ptr<Node>& node);
        void process(const std::shared_ptr<Node>& node, float alpha);
        void physicsProcess(const std::shared_ptr<Node>& node, float delta);
        void input(const std::shared_ptr<Node>& node, InputEvent& event);

        static Application& get();

    public:
        Application(const Application&) = delete;
        Application &operator=(const Application&) = delete;
        Application(const Application&&) = delete;
        Application &&operator=(const Application&&) = delete;

        static JPH::BodyInterface& _getBodyInterface() { return get().physicsSystem.GetBodyInterface(); }
    };

}