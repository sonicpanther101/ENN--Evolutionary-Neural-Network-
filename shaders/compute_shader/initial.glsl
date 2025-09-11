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

void main() {
    uint index = uint(gl_GlobalInvocationID.x);
    
    if (index >= objects.length() || index == 2) return;

    // 3. Calculate inertial position/y
    objects[index].inertialPosition = (objects[index].position + u_deltaTime * objects[index].velocity + u_deltaTime * u_deltaTime * objects[index].acceleration);

    // 4. initial guess with adaptive initialization / x
    // objects[index].position = 
}