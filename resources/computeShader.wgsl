@group(0) @binding(0) var inputTexture: texture_1d<f32>;
@group(0) @binding(1) var outputTexture: texture_storage_1d<r32float, write>;

@compute @workgroup_size(32)
fn computeStuff(@builtin(global_invocation_id) id: vec3<u32>) {
    // Apply the function f to the buffer element at index id.x:
    let x = textureLoad(inputTexture, id.x, 0);
    textureStore(outputTexture, id.x, vec4<f32>(x.x, 0.0f, 0.0f, 0.0f));
}

fn f(value: f32) -> f32{
    return value + 2.0f;
}