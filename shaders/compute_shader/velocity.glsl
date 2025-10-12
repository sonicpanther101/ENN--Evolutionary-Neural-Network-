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

layout(std430, binding = 0) restrict buffer ObjectBuffer {
    PhysicsObject objects[];
};

layout(location = 0) uniform float u_deltaTime;

void main() {
    uint index = uint(gl_GlobalInvocationID.x);
    
    if (index >= objects.length() || index == 2) return;

    // 37. Update velocity
    // objects[index].velocity = (objects[index].position - objects[index].oldPosition) / u_deltaTime;
    objects[index].oldPosition = objects[index].position;
}