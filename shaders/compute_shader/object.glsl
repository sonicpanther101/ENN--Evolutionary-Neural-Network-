#version 430 core

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

struct PhysicsObject {
    vec4 position;
    vec4 velocity;
    vec4 acceleration;
    float mass;
    float radius;
    float _pad[2];
    vec4 newPosition;
    vec4 oldPosition;
    vec4 inertialPosition;
};

struct Constraint {
    int type;
    int indexA;
    int indexB;
    int _pad;
    float restLength;
    float lambda; // λ_j^(n)
    float stiffness; // k_j^(n)
    float maxStiffness; // k_j^*
    // Could also add min/max bounds for inequality constraints
};

layout(std430, binding = 0) restrict buffer ObjectBuffer {
    PhysicsObject objects[];
};

layout(std430, binding = 1) restrict buffer ConstraintBuffer {
    Constraint constraints[];
};

layout(location = 0) uniform float u_deltaTime;
layout(location = 1) uniform int u_constraint_count;

float DistanceConstraint(vec3 X, vec3 Y, float restLength) {
    return distance(X, Y) - restLength;
}

void main() {
    uint index = uint(gl_GlobalInvocationID.x);
    
    if (index >= objects.length() || index == 2) return;

    // set alpha
    float alpha = 0.95;

    // For point objects - mass matrix M𝑖 = 𝑚𝑖I
    float mass = objects[index].mass;
    mat3 Mass = mass * mat3(1.0);

    // 10. Calculate the force required to have moved the obect by the amount it moved
    vec3 force = -(Mass / (u_deltaTime * u_deltaTime)) * (objects[index].position - objects[index].inertialPosition).xyz;
    // 11. Initialize the local hessian matrix
    mat3 LocalHessian = Mass / (u_deltaTime * u_deltaTime);

    // 12. Iterate over all constraints affecting this object
    for (uint i = 0; i < u_constraint_count; i++) {
        if (!(constraints[i].indexA == index || constraints[i].indexB == index)) continue;

        vec3 otherX = (index == constraints[i].indexA) ? objects[constraints[i].indexB].position.xyz : objects[constraints[i].indexA].position.xyz;
        vec3 dir = objects[index].position.xyz - otherX;
        float currentDistance = DistanceConstraint(objects[index].position.xyz, otherX, constraints[i].restLength);

        float C_star = currentDistance; // Original constraint
        float C_prev = DistanceConstraint(objects[index].oldPosition.xyz, 
                                   (index == constraints[i].indexA) ? 
                                   objects[constraints[i].indexB].oldPosition.xyz : 
                                   objects[constraints[i].indexA].oldPosition.xyz,
                                   constraints[i].restLength);
        float C_regularized = C_star - alpha * C_prev;
        
        // direction of the constraint δCⱼ/δxⱼ
        vec3 constraint_gradient = (length(dir) > 1e-6) ? normalize(dir) : vec3(0.0, 1.0, 0.0);

        // 13. check if hard constraint
        if (constraints[i].type == 1) { // hard constraint
            // 14. Hard constraint C_j(x)
            // force of the constraint
            float constraint_force = constraints[i].stiffness * (C_regularized) + constraints[i].lambda; // to be clamped
            // clamping values and adding the direction of the constraint
            force -= constraint_force * constraint_gradient;

        // 15. check if soft constraint
        } else { // soft constraint
            // 16. Constraint
            // direction of the constraint δCⱼ/δxⱼ
            // force of the constraint
            float constraint_force = constraints[i].stiffness * C_regularized;
            // clamping values and adding the direction of the constraint
            force -= constraint_force * constraint_gradient;
        }

        // 18. Update the local hessian matrix missing geometric stiffness matrix
        // G_ij = λ_j * ∂²C_j/∂x_i²
        mat3 geometric_stiffness = mat3(0.0);
        if (constraints[i].type == 1) { // Only for hard constraints
            float lambda_plus = constraints[i].stiffness * C_regularized + constraints[i].lambda;
            
            // Diagonal approximation based on constraint geometry
            // For distance constraint: geometric term ≈ λ/|dir| * (I - dir⊗dir)
            if (length(dir) > 1e-6) {
                mat3 outer = outerProduct(constraint_gradient, constraint_gradient);
                mat3 G_exact = (lambda_plus / length(dir)) * (mat3(1.0) - outer);
                
                // Take diagonal approximation (norm of columns)
                vec3 diag_vals = vec3(
                    length(G_exact[0]),
                    length(G_exact[1]),
                    length(G_exact[2])
                );
                geometric_stiffness = mat3(
                    diag_vals.x, 0, 0,
                    0, diag_vals.y, 0,
                    0, 0, diag_vals.z
                );
            }
        }

        LocalHessian += constraints[i].stiffness * outerProduct(constraint_gradient, constraint_gradient) + geometric_stiffness;
    }

    // 20. Apply force to objects position
    float det = determinant(LocalHessian);
    if (abs(det) < 1e-6) {
        return; // skip this iteration, matrix not invertible
    }
    vec3 delta_x_i = inverse(LocalHessian) * force;

    // 23. Update position
    if (any(isnan(objects[index].position + vec4(delta_x_i, 0)))) {
        return; // or some safe fallback
    }
    objects[index].newPosition = objects[index].position + vec4(delta_x_i, 0);
}