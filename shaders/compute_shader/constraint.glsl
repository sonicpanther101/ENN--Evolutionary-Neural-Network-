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

float DistanceConstraint(vec3 X, vec3 Y, float restLength) {
    return distance(X, Y) - restLength;
}

void main() {
    uint index = uint(gl_GlobalInvocationID.x);

    float beta = 10.0;

    vec3 Xa = objects[constraints[index].indexA].position.xyz;
    vec3 Xb = objects[constraints[index].indexB].position.xyz;

    float currentDistance = DistanceConstraint(Xa, Xb, constraints[index].restLength);

    // 27. Check if hard constraint
    if (constraints[index].type == 1) { // hard constraint
        // 28. Update lambda
        constraints[index].lambda = constraints[index].stiffness * currentDistance + constraints[index].lambda; // to be clamped

        // 29. check lambda is within min/max bounds
        if (0 < constraints[index].lambda && constraints[index].lambda < 1000000000) {

            // 30. update stiffness
            constraints[index].stiffness += beta * abs(currentDistance);
        }
    } else { // soft / other constraint types
        // 33. update stiffness, but cap at max stiffness
        constraints[index].stiffness = min(constraints[index].maxStiffness, constraints[index].stiffness + beta * abs(currentDistance));
    }

    // constraints[j].lambda = constraint_force; // max(constraint_force, lambda_min);
    // 29. Check lambda against bounds
    // if (lambda_min < lambda) {
        // 30. Update stiffness
        // constraints[j].stiffness += beta * abs(constraints[j].stiffness * currentDistance + constraints[j].lambda);
    // }
}