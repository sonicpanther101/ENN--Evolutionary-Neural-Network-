#version 430 core

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

layout(std430, binding = 0) readonly buffer ObjectBuffer {
    PhysicsObject objects[];
};

layout(std430, binding = 1) readonly buffer ConstraintBuffer {
    Constraint constraints[];
};

uniform mat4 u_projection;

out float v_stress;

void main() {
    Constraint c = constraints[gl_InstanceID];
    PhysicsObject a = objects[c.indexA];
    PhysicsObject b = objects[c.indexB];

    // Choose start (vertex 0) or end (vertex 1) of line
    vec2 pos = (gl_VertexID == 0) ? a.position.xy : b.position.xy;

    // Stress = relative deviation from rest length
    float currentLength = distance(a.position.xy, b.position.xy);
    v_stress = (currentLength - c.restLength) / c.restLength;

    gl_Position = u_projection * vec4(pos, 0.0, 1.0);
}