#include "z0/application.hpp"
#include "z0/nodes/mesh_instance.hpp"

class Cube: public z0::MeshInstance {
public:
    Cube(std::shared_ptr<z0::Mesh> mesh): z0::MeshInstance(mesh) {}

    void onReady() override {
        speed = static_cast<float>(std::rand() % 8) / 4.0f;
    }

    void onProcess(float delta) override {
        float angle = delta * glm::radians(90.0f) * speed;
        transform.rotation = { 0.0f, angle, angle };
    }
private:
    float speed{0.0f};
};

class RootNode: public z0::Node {
public:
    void onReady() override {
        std::shared_ptr<z0::Mesh> mesh1 = std::make_shared<z0::Mesh>("models/cube.obj",
                                         std::make_shared<z0::ImageTexture>("models/cube_diffuse.png"));
        std::shared_ptr<z0::Mesh> mesh2 = std::make_shared<z0::Mesh>("models/sphere.obj",
                                         std::make_shared<z0::ImageTexture>("models/sphere_diffuse.png"));
        std::shared_ptr<Node> rootNode = std::make_shared<Node>();
        node1 = std::make_shared<Cube>(mesh1);
        node1->transform.position = { -1.5f, 0.0f, 0.0f };
        addChild(node1);

        node3 = std::make_shared<Cube>(mesh1);
        node3->transform.position = { 1.0f, 0.0f, 3.0f };
        addChild(node3);

        node2 = std::make_shared<z0::MeshInstance>(mesh2);
        node2->transform.position = { 1.5f, 0.0f, 0.0f };
        addChild(node2);
    }

    void onProcess(float delta) override {
        float angle = delta * glm::radians(90.0f) / 2;
        node2->transform.rotation = { 0.0f, -angle, angle };
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
    z0::Application app{applicationConfig};
    app.start(std::make_shared<RootNode>());
    return 0;
}
