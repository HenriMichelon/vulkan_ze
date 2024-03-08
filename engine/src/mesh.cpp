#include "z0/mesh.hpp"
#include "z0/mainloop.hpp"

#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/util.hpp>

namespace z0 {

    Mesh::Mesh(std::filesystem::path filename) {
        loadFromFile(filename);
    }

    // https://fastgltf.readthedocs.io/v0.7.x/overview.html
    // https://github.com/vblanco20-1/vulkan-guide/blob/all-chapters-1.3-wip/chapter-5/vk_loader.cpp
    void Mesh::loadFromFile(std::filesystem::path filename) {
        std::filesystem::path filepath = Application::getDirectory() / filename;
        fastgltf::Parser parser;
        fastgltf::GltfDataBuffer data;
        data.loadFromFile(filepath);
        auto asset = parser.loadGltf(&data, filepath.parent_path(), fastgltf::Options::None);
        if (auto error = asset.error(); error != fastgltf::Error::None) {
            die(getErrorMessage(error));
        }
        fastgltf::Asset gltf = std::move(asset.get());

        materials.clear();
        materials.push_back(std::make_shared<StandardMaterial>());

        vertices.clear();
        indices.clear();
        for (fastgltf::Mesh& mesh : gltf.meshes) {
            std::cout << mesh.name << std::endl;
            for (auto&& p : mesh.primitives) {
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
                                                                      Vertex newvtx;
                                                                      newvtx.position = v;
                                                                      newvtx.normal = { 1, 0, 0 };
                                                                      newvtx.color = glm::vec4 { 1.f };
                                                                      newvtx.uv.x = 0;
                                                                      newvtx.uv.y = 0;
                                                                      vertices[initial_vtx + index] = newvtx;
                                                                  });
                }
                // load vertex normals
                auto normals = p.findAttribute("NORMAL");
                if (normals != p.attributes.end()) {
                    fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, gltf.accessors[(*normals).second],
                                                                  [&](glm::vec3 v, size_t index) {
                                                                      vertices[initial_vtx + index].normal = v;
                                                                  });
                }
                // load UVs
                auto uv = p.findAttribute("TEXCOORD_0");
                if (uv != p.attributes.end()) {
                    fastgltf::iterateAccessorWithIndex<glm::vec2>(gltf, gltf.accessors[(*uv).second],
                                                                  [&](glm::vec2 v, size_t index) {
                                                                      vertices[initial_vtx + index].uv.x = v.x;
                                                                      vertices[initial_vtx + index].uv.y = v.y;
                                                                  });
                }
                // load vertex colors
                auto colors = p.findAttribute("COLOR_0");
                if (colors != p.attributes.end()) {
                    fastgltf::iterateAccessorWithIndex<glm::vec4>(gltf, gltf.accessors[(*colors).second],
                                                                  [&](glm::vec4 v, size_t index) {
                                                                      vertices[initial_vtx + index].color = v;
                                                                  });
                }
            }
        }
        vulkanModel = std::make_shared<VulkanModel>(Application::getViewport()._getDevice(), vertices, indices);
    }


}