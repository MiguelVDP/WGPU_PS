struct stepData{
    num_dof: u32,
    time_step: f32,
};

@group(0) @binding(0) var<storage, read_write> x_buffer: array<f32>;
@group(0) @binding(1) var<storage, read_write> pf_buffer: array<f32>;
@group(0) @binding(2) var<storage, read_write> v_buffer: array<f32>;
@group(0) @binding(3) var<storage, read> step_data: stepData;

@compute @workgroup_size(128)
fn update_V_X(@builtin(global_invocation_id) id: vec3<u32>) {
    if(id.x > step_data.num_dof){
        return;
    }

    let ts = step_data.time_step;

    //Compute the new v
    v_buffer[id.x] = (pf_buffer[id.x] - x_buffer[id.x]) / ts;

    x_buffer[id.x] = pf_buffer[id.x];
}