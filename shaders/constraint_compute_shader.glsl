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
layout(location = 3) uniform int u_objectCount;
layout(location = 4) uniform int u_constraintCount;

float DistanceConstraint(vec3 X, vec3 Y, float restLength) {
    return distance(X, Y) - restLength;
}

// 26. loop over all constraints, TODO: needs to be done seperately to object parallelization
void main() {
    uint index = uint(gl_GlobalInvocationID.x);

    float beta = 10.0;

    // 28. Update lambda
    vec3 currentX = objects[constraints[index].indexA].position.xyz;
    vec3 otherX = objects[constraints[index].indexB].position.xyz;
    float currentDistance = DistanceConstraint(currentX, otherX, constraints[index].restLength);
    float constraint_force = constraints[index].stiffness * currentDistance;

    constraints[index].lambda = constraint_force; // max(constraint_force, lambda_min);
    // 29. Check lambda against bounds
    // if (lambda_min < lambda) {
        // 30. Update stiffness
        constraints[index].stiffness += beta * abs(constraints[index].stiffness * currentDistance + constraints[index].lambda);
    // }
}