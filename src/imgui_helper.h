#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <vector>
#include <string>

#include "gpu_physics.h"

class Imgui {
public:
    void Init(GLFWwindow* window);
    void NewFrame();
    void Render();
    void AddElements(GPUPhysicsSystem physics_system, std::vector<GPUPhysicsObject> physics_data, float dt);
    void Cleanup();
};