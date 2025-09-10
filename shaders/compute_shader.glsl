#version 430 core

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

struct PhysicsObject {
    vec4 position;
    vec4 velocity;
    vec4 acceleration;
    float mass;
    float radius;
};

struct Constraint {
    int type;
    int indexA;
    int indexB;
    float restLength;
    float stiffness; // k_j^(n)
    float lambda; // λ_j^(n)
    // Could also add min/max bounds for inequality constraints
};

layout(std430, binding = 0) restrict buffer ObjectBuffer {
    PhysicsObject objects[];
};

layout(std430, binding = 1) restrict buffer ConstraintBuffer {
    Constraint constraints[];
};

layout(location = 0) uniform float u_deltaTime;
layout(location = 1) uniform int u_iterations;
layout(location = 2) uniform vec2 u_screenSize;

float DistanceConstraint(vec3 X) {
    return distance(X, vec3(u_screenSize.x/2, u_screenSize.y/2, 0)) - u_screenSize.y/4;
}

void main() {
    uint index = uint(gl_GlobalInvocationID.x);
    
    if (index >= objects.length() || index == 2) return;

    // Initialize constraint variables
    float lambda = 0.0;
    // float lambda_min = 0.0; // paper says to set as 0 but that doesn't work
    // float lambda_max = 100.0; // paper says to set to infinity
    float stiffness = 1.0;
    float beta = 10.0;

    // For point objects - mass matrix M_i
    float mass = objects[index].mass;
    mat3 Mass = mass * mat3(1.0);

    // 3. Calculate new position/y
    vec3 y = (objects[index].position + u_deltaTime * objects[index].velocity + u_deltaTime * u_deltaTime * objects[index].acceleration).xyz;

    // Store previous position
    vec3 initialX = objects[index].position.xyz;
    // Store current position
    vec3 currentX = objects[index].position.xyz;

    // 7. Iterate n times
    for (int i = 0; i < u_iterations; i++) {
        // 10. Calculate the force required to have moved the obect by the amount it moved
        vec3 force = -(Mass / (u_deltaTime * u_deltaTime)) * (currentX - y);
        // 11. Initialize the local hessian matrix
        mat3 LocalHessian = Mass / (u_deltaTime * u_deltaTime);

        // 9. Iterate over all colors
        for (uint j = 0; j < 1; j++) {

            // 14. Hard constraint C_j(x)
            float currentDistance = DistanceConstraint(currentX);
            // direction of the constraint δCⱼ/δxⱼ
            vec3 dir = currentX - vec3(u_screenSize.x/2, u_screenSize.y/2, 0);
            vec3 constraint_gradient = (length(dir) > 1e-6) ? normalize(dir) : vec3(0.0, 1.0, 0.0);
            // force of the constraint
            float constraint_force = stiffness * currentDistance + lambda;
            // clamping values and adding the direction of the constraint
            force -= constraint_force * constraint_gradient;

            // 18. Update the local hessian matrix missing geometric stiffness matrix
            LocalHessian += stiffness * outerProduct(constraint_gradient, constraint_gradient);

            // 20. Apply force to objects position
            float det = determinant(LocalHessian);
            if (abs(det) < 1e-6) {
                continue; // skip this iteration, matrix not invertible
            }
            vec3 delta_x_i = inverse(LocalHessian) * force;
            currentX += delta_x_i;

            // 23. Update position
            if (any(isnan(currentX))) {
                currentX = initialX; // or some safe fallback
            }
            objects[index].position = vec4(currentX, 1.0);
        }
    }

    // 37. Update velocity
    objects[index].velocity = vec4((objects[index].position.xyz - initialX) / u_deltaTime, 0.0);
}