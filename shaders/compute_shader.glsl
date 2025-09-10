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

float DistanceConstraint(vec3 X, vec3 Y, float restLength) {
    return distance(X, Y) - restLength;
}

void main() {
    uint index = uint(gl_GlobalInvocationID.x);
    
    if (index >= objects.length() || index == 2) return;

    // Initialize constraint variables

    // float lambda_min = 0.0; // paper says to set as 0 but that doesn't work
    // float lambda_max = 100.0; // paper says to set to infinity

    // stiffness update multiplier
    float beta = 0.1;
    // error tolerance α ∈ [0, 1]
    float alpha = 0.95;

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
        // 9. Iterate over all colors
        for (uint j = 0; j < 1; j++) {

            // 10. Calculate the force required to have moved the obect by the amount it moved
            vec3 force = -(Mass / (u_deltaTime * u_deltaTime)) * (currentX - y);
            // 11. Initialize the local hessian matrix
            mat3 LocalHessian = Mass / (u_deltaTime * u_deltaTime);

                // 12. Iterate over all constraints affecting this object
                for (uint k = 0; k < 1; k++) {
                    if (!(constraints[k].indexA == index || constraints[k].indexB == index)) continue;

                    vec3 otherX = (index == constraints[k].indexA) ? objects[constraints[k].indexB].position.xyz : objects[constraints[k].indexA].position.xyz;
                    vec3 dir = (index == constraints[k].indexA) ? (currentX - otherX) : (otherX - currentX);
                    float currentDistance = DistanceConstraint(currentX, otherX, constraints[k].restLength);
                    // direction of the constraint δCⱼ/δxⱼ
                    vec3 constraint_gradient = (length(dir) > 1e-6) ? normalize(dir) : vec3(0.0, 1.0, 0.0);

                    // 13. check if hard constraint
                    if (constraints[k].type == 1) { // hard constraint
                        // 14. Hard constraint C_j(x)
                        // force of the constraint
                        float constraint_force = constraints[k].stiffness * (currentDistance) + constraints[k].lambda;
                        // clamping values and adding the direction of the constraint
                        force -= constraint_force * constraint_gradient;

                    // 15. check if soft constraint
                    } else { // soft constraint
                        // 16. Constraint
                        // direction of the constraint δCⱼ/δxⱼ
                        // force of the constraint
                        float constraint_force = constraints[k].stiffness * currentDistance;
                        // clamping values and adding the direction of the constraint
                        force -= constraint_force * constraint_gradient;
                    }

                    // 18. Update the local hessian matrix missing geometric stiffness matrix
                    mat3 exact_geometric_stiffness = constraints[k].lambda/length(dir) * (mat3(1.0)  - outerProduct(normalize(dir), normalize(dir)));

                    mat3 diag_approx = mat3(
                        length(exact_geometric_stiffness[0]), 0, 0,
                        0, length(exact_geometric_stiffness[1]), 0,
                        0, 0, length(exact_geometric_stiffness[2])
                    );

                    LocalHessian += constraints[k].stiffness * outerProduct(constraint_gradient, constraint_gradient) + diag_approx;
                }

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

        // 26. loop over all constraints
        for (uint j = 0; j < 1; j++) {
            if (!(constraints[j].indexA == index || constraints[j].indexB == index)) continue;
            // 28. Update lambda
            vec3 otherX = (index == constraints[j].indexA) ? objects[constraints[j].indexB].position.xyz : objects[constraints[j].indexA].position.xyz;
            float currentDistance = DistanceConstraint(currentX, otherX, constraints[j].restLength);
            float constraint_force = constraints[j].stiffness * currentDistance;

            // constraints[j].lambda = constraint_force; // max(constraint_force, lambda_min);
            // 29. Check lambda against bounds
            // if (lambda_min < lambda) {
                // 30. Update stiffness
                // constraints[j].stiffness += beta * abs(constraints[j].stiffness * currentDistance + constraints[j].lambda);
            // }
        }
    }

    // 37. Update velocity
    objects[index].velocity = vec4((objects[index].position.xyz - initialX) / u_deltaTime, 0.0);
}