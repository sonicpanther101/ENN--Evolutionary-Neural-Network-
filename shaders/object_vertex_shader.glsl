#version 430 core

layout(location = 0) in vec2 template_pos;

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

layout(std430, binding = 0) restrict readonly buffer ObjectBuffer {
    PhysicsObject objects[];
};

uniform mat4 u_projection;

out vec3 v_color;

void main() {
    PhysicsObject obj = objects[gl_InstanceID];
    
    vec2 world_pos;
    
    // circle
    world_pos = obj.position.xy + template_pos * obj.radius;
    
    gl_Position = u_projection * vec4(world_pos, 0.0, 1.0);
    v_color = vec3(1.0);
}