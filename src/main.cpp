// src/main.cpp - REPLACE the existing main.cpp with this
#include "gpu_physics.h"
#include "window.h"
#include <chrono>
#include <iostream>

int main() {
    // Initialize window
    Window window(800, 600, "GPU Physics 2D Renderer");
    
    // Check for compute shader support
    int work_group_count[3];
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_group_count[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_group_count[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_group_count[2]);
    
    std::cout << "Max compute work groups: " << work_group_count[0] << ", " 
              << work_group_count[1] << ", " << work_group_count[2] << std::endl;
    
    // Initialize GPU physics system
    GPUPhysicsSystem physics_system(100); // Start with fewer objects for testing
    GPURenderer2D renderer(800, 600);
    
    // Create some bouncing balls
    for (int i = 0; i < 10; ++i) {
        GPUPhysicsObject ball = {};
        ball.position = {10.0f + i * 5.0f, 500.0f};
        ball.velocity = {50.0f + i * 0.1f, -30.0f};
        ball.acceleration = {0.0f, -98.0f}; // gravity
        // ball.radius = 20.0f;
        ball.color = {1.0f, 1.0f, 1.0f};
        ball.shape_type = 2; // circle
        ball.sides = 3;
        ball.position2 = {1.0f, 0.0f};
        ball.filled = 0;
        
        physics_system.addObject(ball);
    }
    
    auto last_time = std::chrono::high_resolution_clock::now();
    
    while (!window.shouldClose()) {
        // Calculate delta time
        auto current_time = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float>(current_time - last_time).count();
        last_time = current_time;
        
        // Cap delta time to avoid large jumps
        dt = std::min(dt, 0.033f); // ~30fps minimum
        
        // Update physics on GPU
        physics_system.update(dt);
        
        // Render directly from GPU buffers
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        renderer.render(physics_system);
        
        window.swapBuffers();
        window.pollEvents();
    }
    
    return 0;
}