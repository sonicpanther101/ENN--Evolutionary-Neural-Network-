#include "gpu_physics.h"
#include "window.h"
#include "imgui_helper.h"
#include <chrono>
#include <iostream>

const int SCREEN_WIDTH = 1600;
const int SCREEN_HEIGHT = 1200;

int main() {
    // Initialize window
    Window window(SCREEN_WIDTH, SCREEN_HEIGHT, "GPU Physics 2D Renderer");

    // Initialize Imgui
    ImguiHelper imgui;
    imgui.Init(window.getGLFWwindow());
    
    // Check for compute shader support
    int work_group_count[3];
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_group_count[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_group_count[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_group_count[2]);
    
    std::cout << "Max compute work groups: " << work_group_count[0] << ", " 
              << work_group_count[1] << ", " << work_group_count[2] << std::endl;
    
    // Initialize GPU physics system
    GPUPhysicsSystem physics_system(100, 100, 10, SCREEN_WIDTH, SCREEN_HEIGHT); // Start with fewer objects for testing
    GPURenderer2D renderer(SCREEN_WIDTH, SCREEN_HEIGHT);
    
    // Create some balls
    GPUPhysicsObject ball = {};
    ball.position = {SCREEN_WIDTH/2+SCREEN_HEIGHT/4, SCREEN_HEIGHT/2, 0.0f, 0.0f};
    ball.velocity = {0.0f, 0.0f, 0.0f, 0.0f};
    ball.acceleration = {0.0f, -100.0f, 0.0f, 0.0f}; // gravity
    ball.mass = 1.0f;
    ball.radius = 20.0f;
    
    physics_system.addObject(ball);

    ball = {};
    ball.position = {SCREEN_WIDTH/2.0f, 3.0f*SCREEN_HEIGHT/4.0f, 0.0f, 0.0f};
    ball.velocity = {1.0f, 0.0f, 0.0f, 0.0f};
    ball.acceleration = {0.0f, -100.0f, 0.0f, 0.0f}; // gravity
    ball.mass = 1.0f;
    ball.radius = 20.0f;
    
    physics_system.addObject(ball);

    // Ball showing correct path
    ball = {};
    ball.position = {SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 0.0f, 0.0f};
    ball.velocity = {0.0f, 0.0f, 0.0f, 0.0f};
    ball.acceleration = {0.0f, 0.0f, 0.0f, 0.0f}; // gravity
    ball.mass = 1.0f;
    ball.radius = SCREEN_HEIGHT/4.0f;
    
    physics_system.addObject(ball);

    for (int i = 0; i < physics_system.getObjectCount(); i++) {
        GPUPhysicsConstraint constraint = {};
        constraint.type = 0;
        constraint.indexA = 2;
        constraint.indexB = i;
        constraint.restLength = SCREEN_HEIGHT/4.0f;
        constraint.stiffness = 1.0f;
        
        physics_system.addConstraint(constraint);   
    }
    
    auto last_time = std::chrono::high_resolution_clock::now();

    // Storage for physics data read back from GPU
    std::vector<GPUPhysicsObject> physics_data;
    
    while (!window.shouldClose()) {
        window.pollEvents();

        imgui.NewFrame();

        // Calculate delta time
        auto current_time = std::chrono::high_resolution_clock::now();
        // float dt = 0.5f;
        float dt = std::chrono::duration<float>(current_time - last_time).count();
        last_time = current_time;
        
        // Cap delta time to avoid large jumps
        dt = std::min(dt, 0.033f); // ~30fps minimum
        
        // Update physics on GPU
        physics_system.update(dt);

        // Read back physics data for ImGui display
        physics_data = physics_system.getObjectsData();

        // Add elements to ImGui window
        imgui.AddElements(&physics_system, physics_data, dt);
        
        // Render directly from GPU buffers
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        renderer.renderObjects(physics_system);
        renderer.renderConstraints(physics_system);

        imgui.Render();
        
        window.swapBuffers();
        window.pollEvents();
    }

    imgui.Cleanup();
    
    return 0;
}