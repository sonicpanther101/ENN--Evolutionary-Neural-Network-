#version 430 core

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

struct PhysicsObject {
    vec4 position;
    vec4 velocity;
    vec4 acceleration;
    float mass;
};

layout(std430, binding = 0) restrict buffer ObjectBuffer {
    PhysicsObject objects[];
};

layout(location = 0) uniform float u_deltaTime;
layout(location = 1) uniform int u_iterations;
layout(location = 2) uniform vec2 u_screenSize;

float DistanceConstraint(vec3 X) {
    return sqrt(dot(X - vec3(u_screenSize.x/2, u_screenSize.y/2, 0), X - vec3(u_screenSize.x/2, u_screenSize.y/2, 0)));
}

void main() {
    uint index = uint(gl_GlobalInvocationID.x);
    
    if (index >= objects.length()) {
        return;
    }

    // 1. Initialize lambda_min and lambda_max
    float lambda_min = 0.0;
    float lambda_max = 100.0;

    // For point objects - mass matrix M_i
    float mass = objects[index].mass;
    mat3 Mass = mass * mat3(1.0);

    // 3. Calculate new position/y
    vec3 y = (objects[index].position + u_deltaTime * objects[index].velocity + u_deltaTime * u_deltaTime * objects[index].acceleration).xyz;

    // Store previous position
    vec3 initialX = objects[index].position.xyz;
    // Store current position
    vec3 currentX = objects[index].position.xyz;

    // 7. Iterate n times
    for (int i = 0; i < u_iterations; i++) {
        // 10. Calculate the force required to have moved the obect by the amount it moved
        vec3 force = -(Mass / (u_deltaTime * u_deltaTime)) * (currentX - y);
        // 11. Initialize the local hessian matrix
        mat3 LocalHessian = Mass / (u_deltaTime * u_deltaTime);

        // 14. Hard constraint force
        float hard_constraint = DistanceConstraint(currentX);
        // clamping values and adding the direction of the constraint δCⱼ/δxⱼ
        force -= max(min(hard_constraint, lambda_max), lambda_min) * normalize(currentX - vec3(u_screenSize/2, 0));

        // 20. Apply force to objects position
        vec3 delta_x_i = inverse(LocalHessian) * force;
        currentX += delta_x_i;
    }

    // Store new position
    objects[index].position = vec4(currentX, 1.0);

    // 37. Update velocity
    objects[index].velocity = vec4((objects[index].position.xyz - initialX) / u_deltaTime, 0.0);
}