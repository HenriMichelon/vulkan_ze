#include "z0/application.hpp"

class ExampleApp : z0::Application {
public:
    ExampleApp(const z0::ApplicationConfig& cfg): z0::Application{cfg} {
        //glm::mat4 rot = glm::rotate(glm::mat4(1.0f), delta * glm::radians(90.0f) / 2, glm::vec3(1.0f, 0.0f, 1.0f));

        /*std::shared_ptr<Mesh> mesh1 = std::make_shared<Mesh>(*this, appdir,  "models/cube.obj",
                                                             std::make_shared<ImageTexture>(*this, appdir, "models/cube_diffuse.png"));
        std::shared_ptr<Mesh> mesh2 = std::make_shared<Mesh>(*this, appdir, "models/sphere.obj",
                                                             std::make_shared<ImageTexture>(*this, appdir, "models/sphere_diffuse.png"));
        std::shared_ptr<Node> rootNode = std::make_shared<Node>();
        auto node1 = std::make_shared<MeshInstance>(mesh1);
        node1->transform.position = { -1.5f, 0.0f, 0.0f };
        rootNode->addChild(node1);

        auto node3 = std::make_shared<MeshInstance>(mesh1);
        node3->transform.position = { 1.0f, 0.0f, 3.0f };
        rootNode->addChild(node3);

        auto node2 = std::make_shared<MeshInstance>(mesh2);
        node2->transform.position = { 1.5f, 0.0f, 0.0f };
        rootNode->addChild(node2);*/
    };
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
    ExampleApp{applicationConfig};
    return 0;
}
