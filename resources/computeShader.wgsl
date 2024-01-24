@group(0) @binding(0) var inputTexture: texture_1d<f32>;
@group(0) @binding(1) var outputTexture: texture_1d<f32>;

@compute @workgroup_size(32)
fn computeStuff(@builtin(global_invocation_id) id: vec3<u32>) {
    // Apply the function f to the buffer element at index id.x:
//    outputBuffer[id.x] = f(inputTexture[id.x]);
}

fn f(value: f32) -> f32{
    return value + 2.0f;
}