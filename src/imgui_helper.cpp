#include "imgui_helper.h"
#define IMPLOT_IMPLEMENTATION
#include "../vendor/implot/implot.h"

void ImguiHelper::Init(GLFWwindow* window) {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
}

void ImguiHelper::NewFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImguiHelper::Render() {
    // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImguiHelper::AddElements(GPUPhysicsSystem* physics_system, std::vector<GPUPhysicsObject> physics_data, float dt) {
    // Create ImGui window for physics tracking
    ImGui::Begin("Physics Object Tracker");
    
    ImGui::Text("Delta Time: %.3f ms", dt * 1000.0f);
    ImGui::Text("FPS: %.0f", 1/dt);
    ImGui::Text("Number of Objects: %zu", physics_data.size());
    ImGui::Separator();
    
    // Calculate total kinetic energy for this frame
    float total_kinetic_energy = 0.0f;
    for (const auto& obj : physics_data) {
        total_kinetic_energy += 0.5f * obj.mass * 
            (obj.velocity.x * obj.velocity.x + 
             obj.velocity.y * obj.velocity.y + 
             obj.velocity.z * obj.velocity.z);
    }
    
    // Update history buffers
    static float accumulated_time = 0.0f;
    accumulated_time += dt;
    
    kinetic_energy_history.push_back(total_kinetic_energy);
    time_history.push_back(accumulated_time);
    
    // Keep history within bounds
    if (kinetic_energy_history.size() > max_history_points) {
        kinetic_energy_history.pop_front();
        time_history.pop_front();
    }
    
    // Display current kinetic energy
    ImGui::Text("Total Kinetic Energy: %.3f J", total_kinetic_energy);
    
    // Kinetic Energy Plot
    if (ImGui::CollapsingHeader("Kinetic Energy Graph", ImGuiTreeNodeFlags_DefaultOpen)) {
        // Convert deques to vectors for plotting (since deques don't have .data())
        std::vector<float> time_vec(time_history.begin(), time_history.end());
        std::vector<float> energy_vec(kinetic_energy_history.begin(), kinetic_energy_history.end());
        
        if (ImPlot::BeginPlot("Kinetic Energy Over Time")) {
            ImPlot::SetupAxes("Time (s)", "Energy (J)");
            ImPlot::SetupAxisLimits(ImAxis_X1, accumulated_time - 10.0f, accumulated_time, ImGuiCond_Always);
            ImPlot::SetupAxisLimits(ImAxis_Y1, 0, total_kinetic_energy * 1.5f, ImGuiCond_Once);
            
            ImPlot::PlotLine("Total Kinetic Energy", 
                            time_vec.data(), 
                            energy_vec.data(), 
                            time_vec.size());
            
            ImPlot::EndPlot();
        }
        
        // Plot controls
        static bool auto_fit = true;
        ImGui::Checkbox("Auto-scale Y", &auto_fit);
        if (auto_fit) {
            ImPlot::SetNextAxesToFit();
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Clear Data")) {
            kinetic_energy_history.clear();
            time_history.clear();
            accumulated_time = 0.0f;
        }
    }
    
    // Create a table for better organization
    if (ImGui::BeginTable("PhysicsObjects", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        // Table headers
        ImGui::TableSetupColumn("Object ID", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn("Position", ImGuiTableColumnFlags_WidthFixed, 150.0f);
        ImGui::TableSetupColumn("Velocity", ImGuiTableColumnFlags_WidthFixed, 150.0f);
        ImGui::TableSetupColumn("Mass", ImGuiTableColumnFlags_WidthFixed, 60.0f);
        ImGui::TableHeadersRow();
        
        // Display each physics object
        for (size_t i = 0; i < physics_data.size(); ++i) {
            const auto& obj = physics_data[i];
            
            ImGui::TableNextRow();
            
            // Object ID
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%zu", i);
            
            // Position
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("(%.1f, %.1f, %.1f)", obj.position.x, obj.position.y, obj.position.z);
            
            // Velocity
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("(%.1f, %.1f, %.1f)", obj.velocity.x, obj.velocity.y, obj.velocity.z);
            
            // Mass
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%.2f", obj.mass);
        }
        
        ImGui::EndTable();
    }
    
    ImGui::Separator();
    
    // Individual object details (expandable)
    for (size_t i = 0; i < physics_data.size(); ++i) {
        const auto& obj = physics_data[i];
        
        if (ImGui::TreeNode(("Object " + std::to_string(i)).c_str())) {
            ImGui::Text("Position:");
            ImGui::SameLine();
            ImGui::Text("X: %.3f, Y: %.3f, Z: %.3f", obj.position.x, obj.position.y, obj.position.z);
            
            ImGui::Text("Velocity:");
            ImGui::SameLine();
            ImGui::Text("X: %.3f, Y: %.3f, Z: %.3f", obj.velocity.x, obj.velocity.y, obj.velocity.z);
            
            ImGui::Text("Speed: %.3f", 
                sqrt(obj.velocity.x * obj.velocity.x + 
                        obj.velocity.y * obj.velocity.y + 
                        obj.velocity.z * obj.velocity.z));
            
            ImGui::Text("Acceleration:");
            ImGui::SameLine();
            ImGui::Text("X: %.3f, Y: %.3f, Z: %.3f", obj.acceleration.x, obj.acceleration.y, obj.acceleration.z);
            
            ImGui::Text("Mass: %.3f", obj.mass);
            
            // Individual object kinetic energy
            float kinetic_energy = 0.5f * obj.mass * 
                (obj.velocity.x * obj.velocity.x + 
                 obj.velocity.y * obj.velocity.y + 
                 obj.velocity.z * obj.velocity.z);
            ImGui::Text("Kinetic Energy: %.3f", kinetic_energy);
            
            ImGui::TreePop();
        }
    }
    
    // Physics system controls
    ImGui::Separator();
    ImGui::Text("Controls:");
    
    static int iterations = 1;
    if (ImGui::SliderInt("Iterations", &iterations, 1, 100)) {
        physics_system->setIterations(iterations);
    }
    
    if (ImGui::Button("Reset Objects")) {
        for (int i = 0; i < 3; ++i) {
            GPUPhysicsObject ball = {};
            ball.position = {400.0f + i * 50.0f, 300.0f + i * 30.0f, 0,0};
            ball.velocity = {(i - 1) * 100.0f, (i - 1) * 80.0f, 0,0};
            ball.acceleration = {0.0f, 300.0f, 0,0};
            ball.mass = 1.0f + i * 0.5f;
            physics_system->addObject(ball);
        }
    }
    
    ImGui::End();
}

void ImguiHelper::Cleanup() {
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    ImPlot::DestroyContext();
}