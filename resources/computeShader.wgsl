@group(0) @binding(0) var<storage, read> piBuffer: array<f32>;
@group(0) @binding(1) var<storage, read_write> pfBuffer: array<f32>;
@group(0) @binding(2) var<storage, read> idxBuffer: array<u32>;
@group(0) @binding(3) var<storage, read> dataBuffer: array<f32>;
@group(0) @binding(4) var<storage, read> dataSize: u32;

@compute @workgroup_size(128)
fn projectStretchConstraint(@builtin(global_invocation_id) id: vec3<u32>) {

    if(id.x >= dataSize){
        return;
    }
    // Load the stencil indices
    let idxA = idxBuffer[id.x * 2u];
    let idxB = idxBuffer[(id.x * 2u) + 1u];

    //Load the stencil positions
    let Ax = piBuffer[idxA];
    let Ay = piBuffer[idxA + 1u];
    let Az = piBuffer[idxA + 2u];
    var nodeA = vec3<f32>(Ax, Ay, Az);

    let Bx = piBuffer[idxB];
    let By = piBuffer[idxB + 1u];
    let Bz = piBuffer[idxB + 2u];
    var nodeB = vec3<f32>(Bx, By, Bz);

    //Load the stencil data
    let len0 = dataBuffer[id.x * 3u];
    let wA = dataBuffer[(id.x * 3u) + 1u];
    let wB = dataBuffer[(id.x * 3u) + 2u];

    //Compute constraint
    var dist = nodeA - nodeB;
    let len = length(dist);
    dist = normalize(dist);

    var correction = ((len - len0) * dist) / (wA + wB);

    var correctionA = wA * correction;
    var correctionB = wB * correction;

    //Load the solution
    pfBuffer[idxA] -= correctionA.x;
    pfBuffer[idxA + 1u] -= correctionA.y;
    pfBuffer[idxA + 2u] -= correctionA.z;

    pfBuffer[idxB] += correctionB.x;
    pfBuffer[idxB + 1u] += correctionB.y;
    pfBuffer[idxB + 2u] += correctionB.z;

}