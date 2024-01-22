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
    @location(2) v_pos: vec4f,
};

@vertex
fn vs_main(@builtin(instance_index) instanceIdx : u32, in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    let ratio = 640.0 / 480.0; // The width and height of the target surface

    out.position = vec4f(in.position.x, (in.position.y * ratio), in.position.z, 1.0);
    if(instanceIdx == u32(1)){
        out.position =  uMVP.projMat * uMVP.viewMat * out.position;
        out.v_pos = uMVP.viewMat * out.position;
        out.normal = (uMVP.viewMat * uMVP.model2Mat * vec4f(in.normal, 0.0)).xyz;
    }else{
        out.position =  uMVP.projMat * uMVP.viewMat * uMVP.modelMat * out.position;
        out.v_pos = uMVP.viewMat * out.position;
        out.normal = (uMVP.viewMat * vec4f(in.normal, 0.0)).xyz;
    }

    out.color = vec3(0.6, 0.0, 0.6); // forward to the fragment shader
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
    let N = normalize(in.normal);
    let pp = in.v_pos.xyz; //Point position in view coordenades
    let pl = vec3(0.0); //Light position in view coordenades
    let L = normalize(pl - pp); //Normalized light direction (inverted)
    let R = reflect(-L, N);
    let V = normalize(-pp);
    let n = 100.0;

    let Ia = vec3(0.1); //Ambient intensiti
    let Ka = in.color; //Ambient object component
    let Il = vec3(1.0); //Light intensity
    let Kd = in.color; //Difuse object component
    let Ks = vec3(0.0, 0.0, 1.0); //Specular object component

    var shading = vec3(0.0);

    shading += Ia * Ka; //Ambient factor
    shading += Il * Kd * max(0.0, dot(L,N)); //Difuse factor
//    shading += Il * Ks * pow(max(0.0, dot(R,V)), n); //Specular factor

//    let lightDirection = vec3f(0.0, 0.0, -1.0);
//    let shading = max(0,dot(-lightDirection, N));
//    let ambient = vec3(0.3, 0.0, 0.3);
//    var color = max(in.color * shading, vec3(0.0));
//    color += ambient;
    return vec4f(shading, 1.0);
}