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
    GPUPhysicsSystem physics_system(100, 2); // Start with fewer objects for testing
    GPURenderer2D renderer(800, 600);
    
    // Create some bouncing triangles
    for (int i = 0; i < 1; ++i) {
        GPUPhysicsObject ball = {};
        ball.position = {400.0f, 300.0f,0};
        ball.velocity = {0.0f, 0.0f,0};
        ball.acceleration = {0.0f, 0.0f,0}; // gravity
        ball.mass = 1.0f;
        
        physics_system.addObject(ball);
    }
    
    auto last_time = std::chrono::high_resolution_clock::now();
    
    while (!window.shouldClose()) {
        // Calculate delta time
        auto current_time = std::chrono::high_resolution_clock::now();
        float dt = 0.5f;
        // float dt = std::chrono::duration<float>(current_time - last_time).count();
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