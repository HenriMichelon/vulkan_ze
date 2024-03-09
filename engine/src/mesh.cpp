#include "z0/mesh.hpp"
#include "z0/mainloop.hpp"

#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/util.hpp>

#include <stb_image.h>

namespace z0 {

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
                    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);
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
                                                                &width, &height, &nrChannels, STBI_rgb_alpha);
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
                                                                           &width, &height, &nrChannels, STBI_rgb_alpha);
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
                                                                           &width, &height, &nrChannels, STBI_rgb_alpha);
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

    Mesh::Mesh(const std::filesystem::path& filename) {
        loadFromFile(filename);
    }

    std::shared_ptr<StandardMaterial>& Mesh::getSurfaceMaterial(uint32_t surfaceIndex) {
        return materials[surfaces[surfaceIndex]->materialIndex];
    }

    void Mesh::setSurfaceMaterial(uint32_t surfaceIndex, std::shared_ptr<StandardMaterial>& material) {
        // TODO : clean unused/duplicates materials, reindex
        uint32_t index = materials.size();
        materials.push_back(material);
        surfaces[surfaceIndex]->materialIndex = index;
    }

    // https://fastgltf.readthedocs.io/v0.7.x/overview.html
    // https://github.com/vblanco20-1/vulkan-guide/blob/all-chapters-1.3-wip/chapter-5/vk_loader.cpp
    void Mesh::loadFromFile(const std::filesystem::path& filename) {
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
        materials.clear();
        for (fastgltf::Material& mat : gltf.materials) {
            std::shared_ptr<StandardMaterial> material = std::make_shared<StandardMaterial>();
            if (mat.pbrData.baseColorTexture.has_value()) {
                std::shared_ptr<Image> image = images[mat.pbrData.baseColorTexture.value().textureIndex];
                material->albedoTexture = std::make_shared<ImageTexture>(image);
            }
            materials.push_back(material);
        }
        if (materials.empty()) {
            materials.push_back(std::make_shared<StandardMaterial>());
        }

        for (fastgltf::Mesh& mesh : gltf.meshes) {
            name += "[" + mesh.name + "]";
            for (auto&& p : mesh.primitives) {
                std::shared_ptr<MeshSurface> surface = std::make_shared<MeshSurface>();
                // load indexes
                {
                    fastgltf::Accessor& indexaccessor = gltf.accessors[p.indicesAccessor.value()];
                    surface->indices.reserve(indexaccessor.count);
                    fastgltf::iterateAccessor<std::uint32_t>(gltf, indexaccessor,
                                                             [&](std::uint32_t idx) {
                                                                 surface->indices.push_back(idx);
                                                             });
                }
                // load vertex positions
                {
                    fastgltf::Accessor& posAccessor = gltf.accessors[p.findAttribute("POSITION")->second];
                    surface->vertices.resize(posAccessor.count);
                    fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, posAccessor,
                                                                  [&](glm::vec3 v, size_t index) {
                                                                      Vertex newvtx;
                                                                      newvtx.position = v;
                                                                      newvtx.normal = { 1, 0, 0 };
                                                                      newvtx.color = glm::vec4 { 1.f };
                                                                      newvtx.uv.x = 0;
                                                                      newvtx.uv.y = 0;
                                                                      surface->vertices[index] = newvtx;
                                                                  });
                }
                // load vertex normals
                auto normals = p.findAttribute("NORMAL");
                if (normals != p.attributes.end()) {
                    fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, gltf.accessors[(*normals).second],
                                                                  [&](glm::vec3 v, size_t index) {
                                                                      surface->vertices[index].normal = v;
                                                                  });
                }
                // load UVs
                auto uv = p.findAttribute("TEXCOORD_0");
                if (uv != p.attributes.end()) {
                    fastgltf::iterateAccessorWithIndex<glm::vec2>(gltf, gltf.accessors[(*uv).second],
                                                                  [&](glm::vec2 v, size_t index) {
                                                                      surface->vertices[index].uv.x = v.x;
                                                                      surface->vertices[index].uv.y = v.y;
                                                                  });
                }
                // load vertex colors
                auto colors = p.findAttribute("COLOR_0");
                if (colors != p.attributes.end()) {
                    fastgltf::iterateAccessorWithIndex<glm::vec4>(gltf, gltf.accessors[(*colors).second],
                                                                  [&](glm::vec4 v, size_t index) {
                                                                      surface->vertices[index].color = v;
                                                                  });
                }

                if (p.materialIndex.has_value()) {
                    surface->materialIndex = p.materialIndex.value();
                };
                surface->_model = std::make_shared<VulkanModel>(
                        Application::getViewport()._getDevice(),
                        surface->vertices,
                        surface->indices);
                surfaces.push_back(surface);
            }
        }
        std::cout << name << std::endl;
    }



}