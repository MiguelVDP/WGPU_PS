#pragma once

#include <structs.h>
#include <object.h>
#include <filesystem>
#include <fstream>
#include <webgpu/webgpu.hpp>
#include <tiny_obj_loader.h>

class ResourceManager {
public:
    // (Just aliases to make notations lighter)
    using path = std::filesystem::path;
    using vec3 = glm::vec3;
    using vec2 = glm::vec2;

    // Load a shader from a WGSL file into a new shader module
    static wgpu::ShaderModule loadShaderModule(const path& path, wgpu::Device device);

    // Load an 3D mesh from a standard .obj file into a vertex data buffer
//    static bool loadGeometryFromObj(const path& path, std::vector<VertexAttributes>& vertexData);

    static bool loadGeometryFromObj(const path& path, std::vector<Object>& objectData);
};