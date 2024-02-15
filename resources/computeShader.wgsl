@group(0) @binding(0) var<storage, read> inputBuffer: array<f32>;
@group(0) @binding(1) var<storage, read_write> outputBuffer: array<f32>;
@group(0) @binding(2) var<storage, read> idxBuffer: array<u32>;
@group(0) @binding(3) var<storage, read> dataBuffer: array<f32>;

@compute @workgroup_size(32)
fn computeStuff(@builtin(global_invocation_id) id: vec3<u32>) {

    var dataSize = (arrayLength(&idxBuffer) / 2u);
    if(id.x >= dataSize){
        return;
    }
    // Load the stencil indices
    let idxA = idxBuffer[id.x * 2u];
    let idxB = idxBuffer[(id.x * 2u) + 1u];

    //Load the stencil positions
    let Ax = inputBuffer[idxA];
    let Ay = inputBuffer[idxA + 1u];
    let Az = inputBuffer[idxA + 2u];
    var nodeA = vec3<f32>(Ax, Ay, Az);

    let Bx = inputBuffer[idxB];
    let By = inputBuffer[idxB + 2u];
    let Bz = inputBuffer[idxB + 1u];
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
    outputBuffer[idxA] += correctionA.x;
    outputBuffer[idxA + 1u] += correctionA.y;
    outputBuffer[idxA + 2u] += correctionA.z;

    outputBuffer[idxB] += correctionB.x;
    outputBuffer[idxB + 1u] += correctionB.y;
    outputBuffer[idxB + 2u] += correctionB.z;

}