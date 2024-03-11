struct stepData{
    num_dof: u32,
    time_step: f32,
};

@group(0) @binding(0) var<storage, read> x_buffer: array<f32>;
@group(0) @binding(1) var<storage, read_write> pf_buffer: array<f32>;
@group(0) @binding(2) var<storage, read_write> v_buffer: array<f32>;
@group(0) @binding(3) var<storage, read> step_data: stepData;
@group(0) @binding(4) var<storage, read> f_buffer: array<f32>;
@group(0) @binding(5) var<storage, read_write> pi_buffer: array<f32>;

@compute @workgroup_size(128)
fn computePredictedPos(@builtin(global_invocation_id) id: vec3<u32>) {
    if(id.x > step_data.num_dof){
        return;
    }

    let ts = step_data.time_step;

    //Compute the new v
    v_buffer[id.x] = v_buffer[id.x] + ts * f_buffer[id.x];

    //Compute the predicted position using the new v
    pi_buffer[id.x] = x_buffer[id.x] + ts * v_buffer[id.x];
    pf_buffer[id.x] = pi_buffer[id.x];
}