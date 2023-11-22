//
// Created by Miguel on 22/10/2023.
//
#include "resourceManager.h"

wgpu::ShaderModule ResourceManager::loadShaderModule(const std::filesystem::path &path, wgpu::Device device) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return nullptr;
    }
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    std::string shaderSource(size, ' ');
    file.seekg(0);
    file.read(shaderSource.data(), size);

    wgpu::ShaderModuleWGSLDescriptor shaderCodeDesc{};
    shaderCodeDesc.chain.next = nullptr;
    shaderCodeDesc.chain.sType = wgpu::SType::ShaderModuleWGSLDescriptor;
    shaderCodeDesc.code = shaderSource.c_str();
    wgpu::ShaderModuleDescriptor shaderDesc{};
    shaderDesc.hintCount = 0;
    shaderDesc.hints = nullptr;
    shaderDesc.nextInChain = &shaderCodeDesc.chain;
    return device.createShaderModule(shaderDesc);
}

bool ResourceManager::loadGeometryFromObj(const ResourceManager::path &path, std::vector<Object> &objectData) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string warn;
    std::string err;

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.string().c_str());

    if (!warn.empty()) {
        std::cout << warn << std::endl;
    }

    if (!err.empty()) {
        std::cerr << err << std::endl;
    }

    if (!ret) {
        return false;
    }

    // Filling in m_vertexData:
    objectData.clear();
    for (const auto &shape: shapes) {
        size_t obj_idx = objectData.size();
        objectData.resize(obj_idx + 1);
        size_t vertex_count = shape.mesh.indices.size();
        objectData[obj_idx].triangles.resize((long)vertex_count);
        objectData[obj_idx].positions.resize((long) vertex_count*3);
        objectData[obj_idx].renderNormals.resize((long) vertex_count*3);
        objectData[obj_idx].simNormals.resize((long) vertex_count*3);
        for (size_t i = 0; i < vertex_count; ++i) {
            const tinyobj::index_t &idx = shape.mesh.indices[i];

            objectData[obj_idx].triangles[(long)i] = idx.vertex_index; //Assume 3 vertex faces (triangles)

            auto eI = (Eigen::Index) (3*i);

            objectData[obj_idx].positions(eI) = attrib.vertices[3 * idx.vertex_index + 0];
            objectData[obj_idx].positions(eI + 1) = attrib.vertices[3 * idx.vertex_index + 1];
            objectData[obj_idx].positions(eI + 2) = attrib.vertices[3 * idx.vertex_index + 2];

            objectData[obj_idx].renderNormals(eI) = attrib.normals[3 * idx.normal_index + 0];
            objectData[obj_idx].renderNormals(eI + 1) = attrib.normals[3 * idx.normal_index + 1];
            objectData[obj_idx].renderNormals(eI + 2) = attrib.normals[3 * idx.normal_index + 2];

            objectData[obj_idx].simNormals(eI) = attrib.normals[3 * idx.normal_index + 0];
            objectData[obj_idx].simNormals(eI + 1) = attrib.normals[3 * idx.normal_index + 1];
            objectData[obj_idx].simNormals(eI + 2) = attrib.normals[3 * idx.normal_index + 2];
        }
    }

    return true;
}


