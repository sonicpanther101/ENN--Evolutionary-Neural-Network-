#version 430 core

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

struct PhysicsObject {
    vec3 position;
    vec3 velocity;
    vec3 acceleration;
    float mass;
};

layout(std430, binding = 0) restrict buffer ObjectBuffer {
    PhysicsObject objects[];
};

uniform float u_deltaTime;
uniform int u_iterations;
uniform vec2 u_screenSize;

void main() {
    uint index = gl_GlobalInvocationID.x;
    
    if (index >= objects.length()) {
        return;
    }

    // For point objects
    mat3 inertialMass = objects[index].mass * mat3(1.0);
    
    // 3. Calculate new position/y
    vec3 y = objects[index].position + u_deltaTime * objects[index].velocity + u_deltaTime * u_deltaTime * objects[index].acceleration;
    vec3 previousX = objects[index].position;

    // 7. Iterate n times
    for (int i = 0; i < u_iterations; i++) {
        // 10. Calculate the force required to have moved the obect by the amount it moved
        vec3 force = -inertialMass/(u_deltaTime*u_deltaTime) * (objects[index].position - y);
        // 11. Initialize the local hessian matrix
        mat3 localHessian = inertialMass/(u_deltaTime*u_deltaTime);

        // 20. Apply force to objects position
        objects[index].position += inverse(localHessian) * force;
    }

    // 37. Update velocity
    objects[index].velocity = (objects[index].position - previousX) / u_deltaTime;
}