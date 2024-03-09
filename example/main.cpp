#include "z0/mainloop.hpp"
#include "z0/nodes/mesh_instance.hpp"

class Cube: public z0::MeshInstance {
public:
    Cube(std::shared_ptr<z0::Mesh> mesh): z0::MeshInstance(mesh) {}

    void onReady() override {
        speed = 1.0; //static_cast<float>(std::rand() % 8) / 4.0f;
    }

    void onProcess(float delta) override {
        float angle = delta * glm::radians(90.0f) * speed;
        transform.rotation = { angle, angle / 2, angle / 4};
    }
private:
    float speed{0.0f};
};

class RootNode: public z0::Node {
public:
    void onReady() override {
        std::shared_ptr<z0::Mesh> meshMulti = std::make_shared<z0::Mesh>("models/multi cube.glb");
        node1 = std::make_shared<Cube>(meshMulti);
        node1->transform.scale = glm::vec3{1.01f};
        addChild(node1);

    }

    void onProcess(float delta) override {
        /*float angle = delta * glm::radians(90.0f) / 2;
        node2->transform.rotation = { 0.0f, -angle, angle };*/
    }

private:
    std::shared_ptr<z0::MeshInstance> node1;
    std::shared_ptr<z0::MeshInstance> node2;
    std::shared_ptr<z0::MeshInstance> node3;
};

int main() {
    z0::ApplicationConfig applicationConfig {
        .appName = "Example App",
        .appDir = "..",
        .windowMode = z0::WINDOW_MODE_WINDOWED,
        .windowWidth = 1024,
        .windowHeight = 768,
        .msaa = z0::MSAA_AUTO
    };
    z0::MainLoop app{applicationConfig};
    app.start(std::make_shared<RootNode>());
    return 0;
}
