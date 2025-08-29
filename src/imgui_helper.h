#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "../vendor/implot/implot.h"

#include <vector>
#include <string>
#include <deque>

#include "gpu_physics.h"

class ImguiHelper {
public:
    void Init(GLFWwindow* window);
    void NewFrame();
    void Render();
    void AddElements(GPUPhysicsSystem* physics_system, std::vector<GPUPhysicsObject> physics_data, float dt);
    void Cleanup();
private:
    glm::vec3 past_velocity = {0.0f, 0.0f, 0.0f};
    std::deque<float> kinetic_energy_history;
    std::deque<float> time_history;
    const size_t max_history_points = 10000;
};