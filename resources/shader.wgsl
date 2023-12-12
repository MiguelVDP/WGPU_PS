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
    @location(1) normal: vec3f,
};

struct VertexOutput {
    @builtin(position) position: vec4f,
    @location(0) color: vec3f,
    @location(1) normal: vec3f,
};

@vertex
fn vs_main(@builtin(instance_index) instanceIdx : u32, in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    let ratio = 640.0 / 480.0; // The width and height of the target surface

    out.position = vec4f(in.position.x, (in.position.y * ratio), in.position.z, 1.0);
    if(instanceIdx == u32(1)){
        out.position =  uMVP.projMat * uMVP.viewMat * uMVP.model2Mat * out.position;
        out.normal = (uMVP.model2Mat * vec4f(in.normal, 0.0)).xyz;
    }else{
        out.position =  uMVP.projMat * uMVP.viewMat * uMVP.modelMat * out.position;
        out.normal = (uMVP.modelMat * vec4f(in.normal, 0.0)).xyz;
    }

    out.color = vec3(0.8, 0.3, 0.6); // forward to the fragment shader
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
    let n = normalize(in.normal);
    let lightDirection = vec3f(0.0, 0.0, -1.0);
    let shading = dot(-lightDirection, n);
    let ambient = vec3(0.3, 0.0, 0.3);
    var color = max(in.color * shading, vec3(0.0));
    color += ambient;
    return vec4f(color, 1.0);
}