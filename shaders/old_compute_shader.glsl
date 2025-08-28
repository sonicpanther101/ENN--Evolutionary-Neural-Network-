#version 430 core

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

struct PhysicsObject {
    vec3 position;
    vec3 velocity;
    vec3 acceleration;
};

layout(std430, binding = 0) restrict buffer ObjectBuffer {
    PhysicsObject objects[];
};

uniform float u_deltaTime;
uniform vec2 u_screenSize;

void main() {
    uint index = gl_GlobalInvocationID.x;
    
    if (index >= objects.length()) {
        return;
    }
    
    // Update velocity
    objects[index].velocity += objects[index].acceleration * u_deltaTime;
    
    // Update position
    objects[index].position += objects[index].velocity * u_deltaTime;
    
    // Boundary collision (simple bounce)
    if (objects[index].position.x - 20 < 0.0 || objects[index].position.x + 20 > u_screenSize.x) {
        objects[index].velocity.x *= -0.8; // damping
        objects[index].position.x = clamp(objects[index].position.x, 20, u_screenSize.x - 20);
    }
    
    if (objects[index].position.y - 20 < 0.0 || objects[index].position.y + 20 > u_screenSize.y) {
        objects[index].velocity.y *= -0.8; // damping  
        objects[index].position.y = clamp(objects[index].position.y, 20, u_screenSize.y - 20);
    }
}