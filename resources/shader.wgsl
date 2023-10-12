//UNIFORMS
// The memory location of the uniform is given by a pair of a *bind group* and a *binding*
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
fn vs_main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    let ratio = 640.0 / 480.0; // The width and height of the target surface
    var offset = vec2f(-0.6875, -0.463);
    offset += 0.3 * vec2f(cos(uTime), sin(uTime));
    let angle = uTime; // you can multiply it go rotate faster
    let alpha = cos(angle);
    let beta = sin(angle);
    var position = vec3f(
        in.position.x,
        alpha * in.position.y + beta * in.position.z,
        alpha * in.position.z - beta * in.position.y,
    );
    out.position = vec4f(position.x, position.y * ratio, position.z * 0.5 + 0.5, 1.0);
    out.color = in.color; // forward to the fragment shader
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
    return vec4f(in.color, 1.0);
}