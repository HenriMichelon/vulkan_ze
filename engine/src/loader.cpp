#include "z0/nodes/mesh_instance.hpp"
#include "z0/mainloop.hpp"
#include "z0/loader.hpp"

#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/util.hpp>

#include <stb_image.h>

namespace z0 {

    // https://fastgltf.readthedocs.io/v0.7.x/tools.html
    // https://github.com/vblanco20-1/vulkan-guide/blob/all-chapters-1.3-wip/chapter-5/vk_loader.cpp
    std::shared_ptr<Image> loadImage(fastgltf::Asset& asset, fastgltf::Image& image) {
        std::shared_ptr<VulkanImage> newImage;
        int width, height, nrChannels;
        std::visit(
            fastgltf::visitor {
                [](auto& arg) {},
                [&](fastgltf::sources::URI& filePath) {
                    assert(filePath.fileByteOffset == 0); // We don't support offsets with stbi.
                    assert(filePath.uri.isLocalPath()); // We're only capable of loading
                    const std::string path(filePath.uri.path().begin(),
                                           filePath.uri.path().end()); // Thanks C++.
                    unsigned char* data = stbi_load(path.c_str(), &width, &height,
                                                    &nrChannels, STBI_rgb_alpha);
                    if (data) {
                        VkDeviceSize imageSize = width * height * STBI_rgb_alpha;
                        newImage = std::make_shared<VulkanImage>(Application::getViewport()._getDevice(),
                                                                 width, height,
                                                                 imageSize, data);

                        stbi_image_free(data);
                    }
                },
                [&](fastgltf::sources::Vector& vector) {
                    unsigned char* data = stbi_load_from_memory(vector.bytes.data(), static_cast<int>(vector.bytes.size()),
                                                                &width, &height,
                                                                &nrChannels, STBI_rgb_alpha);
                    if (data) {
                        VkDeviceSize imageSize = width * height * STBI_rgb_alpha;
                        newImage = std::make_shared<VulkanImage>(Application::getViewport()._getDevice(),
                                                                 width, height,
                                                                 imageSize, data);
                        stbi_image_free(data);
                    }
                },
                [&](fastgltf::sources::BufferView& view) {
                    auto& bufferView = asset.bufferViews[view.bufferViewIndex];
                    auto& buffer = asset.buffers[bufferView.bufferIndex];

                    std::visit(fastgltf::visitor {
                           // We only care about VectorWithMime here, because we
                           // specify LoadExternalBuffers, meaning all buffers
                           // are already loaded into a vector.
                           [](auto& arg) {},
                           [&](fastgltf::sources::Vector& vector) {
                               unsigned char* data = stbi_load_from_memory(vector.bytes.data() + bufferView.byteOffset,
                                                                           static_cast<int>(bufferView.byteLength),
                                                                           &width, &height,
                                                                           &nrChannels, STBI_rgb_alpha);
                               if (data) {
                                   VkDeviceSize imageSize = width * height * STBI_rgb_alpha;
                                   newImage = std::make_shared<VulkanImage>(Application::getViewport()._getDevice(),
                                                                            width, height,
                                                                            imageSize, data);
                                   stbi_image_free(data);
                               }
                           },
                           [&](fastgltf::sources::Array& array) {
                               unsigned char* data = stbi_load_from_memory(array.bytes.data() + bufferView.byteOffset,
                                                                           static_cast<int>(bufferView.byteLength),
                                                                           &width, &height,
                                                                           &nrChannels, STBI_rgb_alpha);
                               if (data) {
                                   VkDeviceSize imageSize = width * height * STBI_rgb_alpha;
                                   newImage = std::make_shared<VulkanImage>(Application::getViewport()._getDevice(),
                                                                            width, height,
                                                                            imageSize, data);
                                   stbi_image_free(data);
                               }
                           },
                           },
                       buffer.data);
                },
            },
        image.data);
        return newImage == nullptr ? nullptr : std::make_shared<Image>(newImage);
    }

    // https://fastgltf.readthedocs.io/v0.7.x/overview.html
    // https://github.com/vblanco20-1/vulkan-guide/blob/all-chapters-1.3-wip/chapter-5/vk_loader.cpp
    std::shared_ptr<Node> Loader::loadModelFromFile(const std::filesystem::path& filename, bool forceBackFaceCulling) {
        std::filesystem::path filepath = Application::getDirectory() / filename;
        fastgltf::Parser parser {};
        constexpr auto gltfOptions =
                fastgltf::Options::DontRequireValidAssetMember |
                fastgltf::Options::AllowDouble |
                fastgltf::Options::LoadGLBBuffers |
                fastgltf::Options::LoadExternalBuffers;
        fastgltf::GltfDataBuffer data;
        data.loadFromFile(filepath);
        auto asset = parser.loadGltfBinary(&data, filepath.parent_path(), gltfOptions);
        if (auto error = asset.error(); error != fastgltf::Error::None) {
            die(getErrorMessage(error));
        }
        fastgltf::Asset gltf = std::move(asset.get());

        // load all textures
        std::vector<std::shared_ptr<Image>> images;
        for (fastgltf::Image& image : gltf.images) {
            images.push_back(loadImage(gltf, image));
        }

        // load all materials
        std::vector<std::shared_ptr<StandardMaterial>> materials{};
        for (fastgltf::Material& mat : gltf.materials) {
            std::shared_ptr<StandardMaterial> material = std::make_shared<StandardMaterial>();
            if (mat.pbrData.baseColorTexture.has_value()) {
                std::shared_ptr<Image> image = images[mat.pbrData.baseColorTexture.value().textureIndex];
                material->albedoTexture = std::make_shared<ImageTexture>(image);
            }
            material->cullMode = forceBackFaceCulling ? CULLMODE_BACK : mat.doubleSided ? CULLMODE_DISABLED : CULLMODE_BACK;
            materials.push_back(material);
        }
        if (materials.empty()) {
            materials.push_back(std::make_shared<StandardMaterial>());
        }

        std::vector<std::shared_ptr<Mesh>> meshes;
        for (fastgltf::Mesh& glftMesh : gltf.meshes) {
            std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>(glftMesh.name.data());
            std::vector<Vertex>& vertices = mesh->getVertices();
            std::vector<uint32_t>& indices = mesh->getIndices();
            for (auto&& p : glftMesh.primitives) {
                std::shared_ptr<MeshSurface> surface = std::make_shared<MeshSurface>(
                        static_cast<uint32_t>(indices.size()),
                        static_cast<uint32_t>(gltf.accessors[p.indicesAccessor.value()].count));
                size_t initial_vtx = vertices.size();
                // load indexes
                {
                    fastgltf::Accessor& indexaccessor = gltf.accessors[p.indicesAccessor.value()];
                    indices.reserve(indices.size() + indexaccessor.count);
                    fastgltf::iterateAccessor<std::uint32_t>(gltf, indexaccessor,
                                                             [&](std::uint32_t idx) {
                                                                 indices.push_back(idx + initial_vtx);
                                                             });
                }
                // load vertex positions
                {
                    fastgltf::Accessor& posAccessor = gltf.accessors[p.findAttribute("POSITION")->second];
                    vertices.resize(vertices.size() + posAccessor.count);
                    fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, posAccessor,
                                                                  [&](glm::vec3 v, size_t index) {
                                                                      Vertex newvtx {
                                                                          .position = v,
                                                                      };
                                                                      vertices[index + initial_vtx] = newvtx;
                                                                  });
                }
                // load vertex normals
                auto normals = p.findAttribute("NORMAL");
                if (normals != p.attributes.end()) {
                    fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, gltf.accessors[(*normals).second],
                                                                  [&](glm::vec3 v, size_t index) {
                                                                      vertices[index + initial_vtx].normal = v;
                                                                  });
                }
                // load UVs
                auto uv = p.findAttribute("TEXCOORD_0");
                if (uv != p.attributes.end()) {
                    fastgltf::iterateAccessorWithIndex<glm::vec2>(gltf, gltf.accessors[(*uv).second],
                                                                  [&](glm::vec2 v, size_t index) {
                                                                      vertices[index + initial_vtx].uv= {
                                                                          v.x,
                                                                          v.y
                                                                      };
                                                                  });
                }
                // load vertex colors
                auto colors = p.findAttribute("COLOR_0");
                if (colors != p.attributes.end()) {
                    fastgltf::iterateAccessorWithIndex<glm::vec4>(gltf, gltf.accessors[(*colors).second],
                                                                  [&](glm::vec4 v, size_t index) {
                                                                      vertices[index + initial_vtx].color = v;
                                                                  });
                }
                // associate material to surface and keep track of all materials used in the Mesh
                if (p.materialIndex.has_value()) {
                    auto material = materials[p.materialIndex.value()];
                    surface->material = material;
                    mesh->_getMaterials().insert(material);
                };
                mesh->getSurfaces().push_back(surface);
            }
            meshes.push_back(mesh);
        }

        // load all nodes and their meshes
        std::vector<std::shared_ptr<Node>> nodes;
        for (fastgltf::Node& node : gltf.nodes) {
            std::shared_ptr<Node> newNode;
            std::string name{node.name.data()};
            // find if the node has a mesh, and if it does hook it to the mesh pointer and allocate it with the meshnode class
            if (node.meshIndex.has_value()) {
                auto mesh = meshes[*node.meshIndex];
                mesh->_buildModel();
                newNode = std::make_shared<MeshInstance>(mesh, name);
            } else {
                newNode = std::make_shared<Node>(name);
            }

            std::visit(fastgltf::visitor { [&](fastgltf::Node::TransformMatrix matrix) {
                           memcpy(&newNode->localTransform, matrix.data(), sizeof(matrix));
                       },
                                           [&](fastgltf::TRS transform) {
                                               glm::vec3 tl(transform.translation[0], transform.translation[1],
                                                            transform.translation[2]);
                                               glm::quat rot(transform.rotation[3], transform.rotation[0], transform.rotation[1],
                                                             transform.rotation[2]);
                                               glm::vec3 sc(transform.scale[0], transform.scale[1], transform.scale[2]);

                                               glm::mat4 tm = glm::translate(glm::mat4(1.f), tl);
                                               glm::mat4 rm = glm::toMat4(rot);
                                               glm::mat4 sm = glm::scale(glm::mat4(1.f), sc);

                                               newNode->localTransform = tm * rm * sm;
                                           } },
                       node.transform);
            newNode->updateTransform(glm::mat4{1.0f});
            nodes.push_back(newNode);
        }

        for (int i = 0; i < gltf.nodes.size(); i++) {
            fastgltf::Node& node = gltf.nodes[i];
            std::shared_ptr<Node>& sceneNode = nodes[i];
            for (auto& c : node.children) {
                sceneNode->addChild(nodes[c]);
            }
        }

        // find the top nodes, with no parents
        std::shared_ptr<Node> rootNode = std::make_shared<Node>(filename.string());
        for (auto& node : nodes) {
            if (node->getParent() == nullptr) {
                rootNode->addChild(node);
            }
        }
        rootNode->rotate({glm::radians(180.f), 0.0f, 0.0f});

        return rootNode;
    }


}