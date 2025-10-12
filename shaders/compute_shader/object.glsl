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
    objects[index].newPosition = objects[index].inertialPosition;
}