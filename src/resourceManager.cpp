//
// Created by Miguel on 22/10/2023.
//
#include "resourceManager.h"
#include "unordered_map"
#include "functional"

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

struct Vertex {
    float x, y, z;

    // Define a hash function for Vertex
    size_t operator()(const Vertex &v) const {
        std::hash<float> hasher;
        size_t hash = 0;
        hash_combine(hash, hasher(v.x));
        hash_combine(hash, hasher(v.y));
        hash_combine(hash, hasher(v.z));
        return hash;
    }

    // Utility function for combining hash values
    template<class T>
    void hash_combine(std::size_t &seed, const T &value) const {
        seed ^= std::hash<T>{}(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    // Define equality for Vertex
    bool operator==(const Vertex &other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};

namespace std {
    template<>
    struct hash<Vertex> {
        size_t operator()(const Vertex &v) const {
            return v(v);
        }
    };
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

        std::vector<uint16_t> triangles;
        std::unordered_map<Vertex, uint16_t> vertexMap;
        std::vector<Vertex> uniqueVertices;
        std::vector<Vertex> uniqueNormals;
        for (const auto &index: shape.mesh.indices) {
            Vertex vertex{};
            vertex.x = attrib.vertices[3 * index.vertex_index + 0];
            vertex.y = attrib.vertices[3 * index.vertex_index + 1];
            vertex.z = attrib.vertices[3 * index.vertex_index + 2];

            // Check if the vertex is already in the map
            auto it = vertexMap.find(vertex);
            if (it == vertexMap.end()) {
                // Vertex not found, add it to the map and unique vertices list
                auto newIndex = static_cast<uint16_t>(uniqueVertices.size());
                vertexMap[vertex] = newIndex;
                uniqueVertices.push_back(vertex);
                triangles.push_back(newIndex);
                // Don´t forget to add the normal
                Vertex normal{};
                normal.x = attrib.normals[3 * index.normal_index + 0];
                normal.y = attrib.normals[3 * index.normal_index + 1];
                normal.z = attrib.normals[3 * index.normal_index + 2];
                uniqueNormals.push_back(normal);
            } else {
                // Vertex found, use its existing index
                triangles.push_back(it->second);
            }
        }

        objectData[obj_idx].triangles.resize((long) triangles.size());
        long vertex_count = (long) uniqueVertices.size();
        objectData[obj_idx].positions.resize(vertex_count * 3);
        objectData[obj_idx].renderNormals.resize(vertex_count * 3);
        objectData[obj_idx].simNormals.resize(vertex_count * 3);

        for (int v = 0; v < (int) vertex_count; v++) {
            objectData[obj_idx].positions[3 * v] = uniqueVertices[v].x;
            objectData[obj_idx].positions[3 * v + 1] = uniqueVertices[v].y;
            objectData[obj_idx].positions[3 * v + 2] = uniqueVertices[v].z;
        }

        for (int n = 0; n < (int) vertex_count; n++) {
            objectData[obj_idx].renderNormals[3 * n] = uniqueNormals[n].x;
            objectData[obj_idx].renderNormals[3 * n + 1] = uniqueNormals[n].y;
            objectData[obj_idx].renderNormals[3 * n + 2] = uniqueNormals[n].z;

            objectData[obj_idx].simNormals[3 * n] = uniqueNormals[n].x;
            objectData[obj_idx].simNormals[3 * n + 1] = uniqueNormals[n].y;
            objectData[obj_idx].simNormals[3 * n + 2] = uniqueNormals[n].z;
        }

        for (int t = 0; t < (int) triangles.size(); t++)
            objectData[obj_idx].triangles[t] = triangles[t];

    }


    return true;
}


//bool ResourceManager::loadGeometryFromObj(const ResourceManager::path &path, std::vector<Object> &objectData) {
//    tinyobj::attrib_t attrib;
//    std::vector<tinyobj::shape_t> shapes;
//    std::vector<tinyobj::material_t> materials;
//
//    std::string warn;
//    std::string err;
//
//    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.string().c_str());
//
//    if (!warn.empty()) {
//        std::cout << warn << std::endl;
//    }
//
//    if (!err.empty()) {
//        std::cerr << err << std::endl;
//    }
//
//    if (!ret) {
//        return false;
//    }
//
//    // Filling in m_vertexData:
//    objectData.clear();
//    for (const auto &shape: shapes) {
//        size_t obj_idx = objectData.size();
//        objectData.resize(obj_idx + 1);
//        size_t vertex_count = shape.mesh.indices.size();
//        objectData[obj_idx].triangles.resize((long)vertex_count);
//        objectData[obj_idx].positions.resize((long) vertex_count*3);
//        objectData[obj_idx].renderNormals.resize((long) vertex_count*3);
//        objectData[obj_idx].simNormals.resize((long) vertex_count*3);
//        for (size_t i = 0; i < vertex_count; ++i) {
//            const tinyobj::index_t &idx = shape.mesh.indices[i];
//
//            objectData[obj_idx].triangles[(long)i] = idx.vertex_index; //Assume 3 vertex faces (triangles)
//
//            auto eI = (Eigen::Index) (3*i);
//
//            objectData[obj_idx].positions(eI) = attrib.vertices[3 * idx.vertex_index + 0];
//            objectData[obj_idx].positions(eI + 1) = attrib.vertices[3 * idx.vertex_index + 1];
//            objectData[obj_idx].positions(eI + 2) = attrib.vertices[3 * idx.vertex_index + 2];
//
//            objectData[obj_idx].renderNormals(eI) = attrib.normals[3 * idx.normal_index + 0];
//            objectData[obj_idx].renderNormals(eI + 1) = attrib.normals[3 * idx.normal_index + 1];
//            objectData[obj_idx].renderNormals(eI + 2) = attrib.normals[3 * idx.normal_index + 2];
//
//            objectData[obj_idx].simNormals(eI) = attrib.normals[3 * idx.normal_index + 0];
//            objectData[obj_idx].simNormals(eI + 1) = attrib.normals[3 * idx.normal_index + 1];
//            objectData[obj_idx].simNormals(eI + 2) = attrib.normals[3 * idx.normal_index + 2];
//        }
//    }
//
//    return true;
//}
//
//
