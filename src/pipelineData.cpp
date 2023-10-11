#include <pipelineData.h>

void PipelineData::setVertexDescription(wgpu::ShaderModule shaderModule, int attribCount) {
    //The shader source code
    pipeDesc.vertex.module = shaderModule;
    //The function entry point in the shader
    pipeDesc.vertex.entryPoint = "vs_main";
    //Vertex constants
    pipeDesc.vertex.constantCount = 0;
    pipeDesc.vertex.constants = nullptr;

    // == Per attribute ==
    attributes.resize(attribCount);
    //Position
    attributes[0].shaderLocation = 0;  // Corresponds to @location(...)
    attributes[0].format = wgpu::VertexFormat::Float32x2;
    attributes[0].offset = 0;
    //Color
    attributes[1].shaderLocation = 1;
    attributes[1].format = wgpu::VertexFormat::Float32x3;
    attributes[1].offset = 2 * sizeof(float);

    // == Common to attributes from the same buffer ==
    vertexBufferLayout.stepMode = wgpu::VertexStepMode::Vertex;
    vertexBufferLayout.attributeCount = static_cast<uint32_t>(attributes.size());
    vertexBufferLayout.attributes = attributes.data();
    vertexBufferLayout.arrayStride = 5 * sizeof(float);
    //Buffers from the information will be sent
    pipeDesc.vertex.bufferCount = 1;
    pipeDesc.vertex.buffers = &vertexBufferLayout;
}

void PipelineData::setPrimitiveDescriptor() {
    // Each sequence of 3 vertices is considered as a triangle
    pipeDesc.primitive.topology = WGPUPrimitiveTopology_TriangleList;

    //Order in which the vertices should be read, if undefined, they will be read sequentially
    pipeDesc.primitive.stripIndexFormat = wgpu::IndexFormat::Undefined;

    //Triangle orientation, (CCW = counter clock wise)
    pipeDesc.primitive.frontFace = WGPUFrontFace_CCW;

    //Set the cull mode (this process avoid painting back faces
    pipeDesc.primitive.cullMode = wgpu::CullMode::None;
}

void PipelineData::setFragmentDescriptor(WGPUTextureFormat swapChainFormat, wgpu::ShaderModule shaderModule) {
    /////////////////////
    /////  FRAGMENT  ////
    /////////////////////
    fragmentState.module = shaderModule;
    fragmentState.entryPoint = "fs_main";
    fragmentState.constantCount = 0;
    fragmentState.constants = nullptr;

    /////////////////////
    /////   BLEND   /////
    /////////////////////

    //As the fragment shader is in charge of colouring the fragments, we should configure the blending (color mixing)
    //Here we use an alpha blending equation
    blendState.color.srcFactor = wgpu::BlendFactor::SrcAlpha;
    blendState.color.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
    blendState.color.operation = wgpu::BlendOperation::Add;
    //We set the alpha channel untouched
    blendState.alpha.srcFactor = wgpu::BlendFactor::Zero;
    blendState.alpha.dstFactor = wgpu::BlendFactor::One;
    blendState.alpha.operation = wgpu::BlendOperation::Add;

    /////////////////////
    //// COLOR TARGET ///
    /////////////////////
    //The color target will define the format and behaviour of the color targets this pipeline writes to
    colorTarget.format = swapChainFormat;
    colorTarget.blend = &blendState;
    colorTarget.writeMask = wgpu::ColorWriteMask::All;

    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;

    pipeDesc.fragment = &fragmentState;
}

void PipelineData::setMisc() {
    pipeDesc.depthStencil = nullptr;

    // Samples per pixel
    pipeDesc.multisample.count = 1;
    // Default value for the mask, meaning "all bits on"
    pipeDesc.multisample.mask = ~0u;
    // Default value as well (irrelevant for count = 1 anyway)
    pipeDesc.multisample.alphaToCoverageEnabled = false;
}

PipelineData::PipelineData() = default;
