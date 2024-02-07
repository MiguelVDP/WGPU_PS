@group(0) @binding(0) var inputTexture: texture_1d<f32>;
@group(0) @binding(1) var outputTexture: texture_storage_1d<r32float, write>;
@group(0) @binding(2) var idxTexture: texture_1d<u32>;
@group(0) @binding(3) var dataTexture: texture_1d<f32>;
@group(0) @binding(4) var<uniform> dataSize: u32;


@compute @workgroup_size(32)
fn computeStuff(@builtin(global_invocation_id) id: vec3<u32>) {

    if(id.x >= dataSize){
        return;
    }
    // Load the stencil indices
    let idxA = textureLoad(idxTexture, (id.x * 2u), 0).x;
    let idxB = textureLoad(idxTexture, (id.x * 2u) + 1u, 0).x;

    //Load the stencil positions
    let Ax = textureLoad(inputTexture, idxA, 0).x;
    let Ay = textureLoad(inputTexture, (idxA + 1u), 0).x;
    let Az = textureLoad(inputTexture, (idxA + 2u), 0).x;
    var nodeA = vec3<f32>(Ax, Ay, Az);

    let Bx = textureLoad(inputTexture, idxB, 0).x;
    let By = textureLoad(inputTexture, (idxB + 1u), 0).x;
    let Bz = textureLoad(inputTexture, (idxB + 2u), 0).x;
    var nodeB = vec3<f32>(Bx, By, Bz);

    //Load the stencil data
    let len0 = textureLoad(dataTexture, (id.x * 3u), 0).x;
    let wA = textureLoad(dataTexture, ((id.x * 3u) + 1u), 0).x;
    let wB = textureLoad(dataTexture, ((id.x * 3u) + 2u), 0).x;

    //Compute constraint
    var dist = nodeA - nodeB;
    let len = length(dist);
    dist = normalize(dist);

    var correction = ((len - len0) * dist) / (wA + wB);

    nodeA -= wA * correction;
    nodeB += wB * correction;

    //Load the solution
    storageBarrier();
    textureStore(outputTexture, idxA, vec4<f32>(nodeA.x, 0.0f, 0.0f, 0.0f));
    storageBarrier();
    textureStore(outputTexture, idxA + 1u, vec4<f32>(nodeA.y, 0.0f, 0.0f, 0.0f));
    storageBarrier();
    textureStore(outputTexture, idxA + 2u, vec4<f32>(nodeA.z, 0.0f, 0.0f, 0.0f));

    storageBarrier();
    textureStore(outputTexture, idxB, vec4<f32>(nodeB.x, 0.0f, 0.0f, 0.0f));
    storageBarrier();
    textureStore(outputTexture, idxB + 1u, vec4<f32>(nodeB.y, 0.0f, 0.0f, 0.0f));
    storageBarrier();
    textureStore(outputTexture, idxB + 2u, vec4<f32>(nodeB.z, 0.0f, 0.0f, 0.0f));

}