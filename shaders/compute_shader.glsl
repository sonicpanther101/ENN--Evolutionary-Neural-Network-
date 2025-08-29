#version 430 core

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

struct PhysicsObject {
    vec4 position;
    vec4 velocity;
    vec4 acceleration;
    float mass;
};

layout(std430, binding = 0) restrict buffer ObjectBuffer {
    PhysicsObject objects[];
};

layout(location = 0) uniform float u_deltaTime;
layout(location = 1) uniform int u_iterations;
layout(location = 2) uniform vec2 u_screenSize;

void main() {
    uint index = uint(gl_GlobalInvocationID.x);
    
    if (index >= objects.length()) {
        return;
    }

    // For point objects - mass matrix M_i
    float mass = objects[index].mass;
    mat3 M_i = mass * mat3(1.0);

    if (u_deltaTime <= 0.0) return;   // prevent div by zero
    if (mass <= 0.0) return;          // skip invalid mass
    
    // 3. Calculate new position/y
    vec3 y = (objects[index].position + u_deltaTime * objects[index].velocity + u_deltaTime * u_deltaTime * objects[index].acceleration).xyz;

    // Store previous position
    vec3 initialX = objects[index].position.xyz;
    // Store current position
    vec3 currentX = objects[index].position.xyz;

    // 7. Iterate n times
    for (int i = 0; i < u_iterations; i++) {
        // 10. Calculate the force required to have moved the obect by the amount it moved
        vec3 f_i = -(M_i / (u_deltaTime * u_deltaTime)) * (currentX - y);
        // 11. Initialize the local hessian matrix
        mat3 H_i = M_i / (u_deltaTime * u_deltaTime);



        // Add force elements (springs, constraints, etc.)
        // For each force element j in F_i affecting vertex i:
        // This is where you would iterate over springs, constraints, contacts, etc.
        // For now, we'll add simple boundary forces as an example
        
        // Simple boundary constraint forces (keep objects within screen bounds)
        float boundary_stiffness = 1000.0;
        
        // Left boundary
        if (currentX.x < 25.0) {
            float constraint_error = currentX.x - 25.0;
            vec3 constraint_gradient = vec3(1.0, 0.0, 0.0);
            
            // Add constraint force (f_ij = -k * C_j * grad_C_j)
            f_i += -boundary_stiffness * constraint_error * constraint_gradient;
            
            // Add to Hessian (H_ij = k * grad_C_j * grad_C_j^T)
            H_i += boundary_stiffness * outerProduct(constraint_gradient, constraint_gradient);
        }
        
        // Right boundary
        if (currentX.x > u_screenSize.x - 25.0) {
            float constraint_error = currentX.x - (u_screenSize.x - 25.0);
            vec3 constraint_gradient = vec3(1.0, 0.0, 0.0);
            
            f_i += -boundary_stiffness * constraint_error * constraint_gradient;
            H_i += boundary_stiffness * outerProduct(constraint_gradient, constraint_gradient);
        }
        
        // Top boundary
        if (currentX.y < 25.0) {
            float constraint_error = currentX.y - 25.0;
            vec3 constraint_gradient = vec3(0.0, 1.0, 0.0);
            
            f_i += -boundary_stiffness * constraint_error * constraint_gradient;
            H_i += boundary_stiffness * outerProduct(constraint_gradient, constraint_gradient);
        }
        
        // Bottom boundary
        if (currentX.y > u_screenSize.y - 25.0) {
            float constraint_error = currentX.y - (u_screenSize.y - 25.0);
            vec3 constraint_gradient = vec3(0.0, 1.0, 0.0);
            
            f_i += -boundary_stiffness * constraint_error * constraint_gradient;
            H_i += boundary_stiffness * outerProduct(constraint_gradient, constraint_gradient);
        }

        // 20. Apply force to objects position
        vec3 delta_x_i = inverse(H_i) * f_i;
        currentX += delta_x_i;
    }

    // Store new position
    objects[index].position = vec4(currentX, 1.0);

    // 37. Update velocity
    objects[index].velocity = vec4((objects[index].position.xyz - initialX) / u_deltaTime, 0.0);
}