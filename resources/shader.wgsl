//UNIFORMS
// The memory location of the uniform is given by a pair of a *bind group* and a *binding*
struct mvpUniforms{
    projMat: mat4x4f,
    viewMat: mat4x4f,
    modelMat: mat4x4f,
    model2Mat: mat4x4f,
};

@group(0) @binding(1) var<uniform> uMVP: mvpUniforms;
@group(0) @binding(0) var<uniform> uTime: f32;


struct VertexInput {
    @location(0) position: vec3f,
    @location(1) color: vec3f,
};

/**
 * A structure with fields labeled with builtins and locations can also be used
 * as *output* of the vertex shader, which is also the input of the fragment
 * shader.
 */
struct VertexOutput {
    @builtin(position) position: vec4f,
    // The location here does not refer to a vertex attribute, it just means
    // that this field must be handled by the rasterizer.
    // (It can also refer to another field of another struct that would be used
    // as input to the fragment shader.)
    @location(0) color: vec3f,
};

@vertex
fn vs_main(@builtin(instance_index) instanceIdx : u32, in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    let ratio = 640.0 / 480.0; // The width and height of the target surface

    out.position = vec4f(in.position.x, (in.position.y * ratio), in.position.z, 1.0);
    if(instanceIdx == u32(1)){
        out.position =  uMVP.projMat * uMVP.viewMat * uMVP.model2Mat * out.position;
    }else{
        out.position =  uMVP.projMat * uMVP.viewMat * uMVP.modelMat * out.position;
    }

    out.color = in.color; // forward to the fragment shader
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
    return vec4f(in.color, 1.0);
}