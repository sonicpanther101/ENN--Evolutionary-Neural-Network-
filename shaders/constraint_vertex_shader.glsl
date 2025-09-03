#version 430 core

struct PhysicsObject {
    vec4 position;
    vec4 velocity;
    vec4 acceleration;
    float mass;
    float radius;
};

struct PhysicsConstraint {
    int type;
    int indexA;
    int indexB;
    float restLength;
    float stiffness;
    float lambda;
};

layout(std430, binding = 0) readonly buffer ObjectBuffer {
    PhysicsObject objects[];
};

layout(std430, binding = 1) readonly buffer ConstraintBuffer {
    PhysicsConstraint constraints[];
};

uniform mat4 u_projection;

out float v_stress;

void main() {
    PhysicsConstraint c = constraints[gl_InstanceID];
    PhysicsObject a = objects[c.indexA];
    PhysicsObject b = objects[c.indexB];

    // Choose start (vertex 0) or end (vertex 1) of line
    vec2 pos = (gl_VertexID == 0) ? a.position.xy : b.position.xy;

    // Stress = relative deviation from rest length
    float currentLength = distance(a.position.xy, b.position.xy);
    v_stress = (currentLength - c.restLength) / c.restLength;

    gl_Position = u_projection * vec4(pos, 0.0, 1.0);
}