#include <GL/glew.h>
#include <GLFW/glfw3.h>

// GLM
#include "../vendor/glm/glm/gtc/type_ptr.hpp"
#include "../vendor/glm/glm/gtc/matrix_transform.hpp"

#include <cstdio>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "shader.h"

#include <iostream>
#include <vector>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// settings
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 800;

bool wireframe = false;
bool showCircles = true;
bool showLines = true;

// 2D positions (x, y coordinates)
std::vector<glm::vec2> circlePositions = {
    glm::vec2(100.0f, 100.0f),
    glm::vec2(300.0f, 200.0f),
    glm::vec2(500.0f, 150.0f),
    glm::vec2(200.0f, 400.0f),
    glm::vec2(600.0f, 350.0f)
};

// Line data (pairs of points)
std::vector<glm::vec2> linePoints = {
    // Connect some circles with lines
    glm::vec2(100.0f, 100.0f), glm::vec2(300.0f, 200.0f),
    glm::vec2(300.0f, 200.0f), glm::vec2(500.0f, 150.0f),
    glm::vec2(200.0f, 400.0f), glm::vec2(600.0f, 350.0f)
};

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main() {

    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "2D Renderer", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSwapInterval(1); // Enable vsync

    // glew: load all OpenGL function pointers
    // ---------------------------------------
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cout << "Failed to initialize GLEW: " << glewGetErrorString(err) << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // build and compile our shader programs
    // ------------------------------------
    Shader circleShader("../shaders/circle_vertex.glsl", "../shaders/circle_fragment.glsl");
    Shader lineShader("../shaders/line_vertex.glsl", "../shaders/line_fragment.glsl");

    // Circle rendering setup
    unsigned int circleVAO, circleVBO, circleInstanceVBO;
    const float circleVertices[] = {
        // Triangle fan for circle (center + edge points)
        0.0f,  0.0f,   // Center
        1.0f,  0.0f,   // Right
        0.866f, 0.5f,  // 
        0.5f,  0.866f, //
        0.0f,  1.0f,   // Top
        -0.5f, 0.866f, //
        -0.866f, 0.5f, //
        -1.0f, 0.0f,   // Left
        -0.866f, -0.5f,//
        -0.5f, -0.866f,//
        0.0f, -1.0f,   // Bottom
        0.5f, -0.866f, //
        0.866f, -0.5f, //
        1.0f,  0.0f    // Back to right
    };

    glGenVertexArrays(1, &circleVAO);
    glGenBuffers(1, &circleVBO);
    glGenBuffers(1, &circleInstanceVBO);

    glBindVertexArray(circleVAO);

    // Vertex buffer for circle shape
    glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(circleVertices), circleVertices, GL_STATIC_DRAW);

    // Vertex attributes for circle shape
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    // Instance buffer for positions
    glBindBuffer(GL_ARRAY_BUFFER, circleInstanceVBO);
    glBufferData(GL_ARRAY_BUFFER, circlePositions.size() * sizeof(glm::vec2), nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glVertexAttribDivisor(1, 1);  // Update once per instance

    glBindVertexArray(0);

    // Line rendering setup
    unsigned int lineVAO, lineVBO;
    glGenVertexArrays(1, &lineVAO);
    glGenBuffers(1, &lineVBO);

    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glBufferData(GL_ARRAY_BUFFER, linePoints.size() * sizeof(glm::vec2), nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glBindVertexArray(0);

    // ImGui setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Update buffers
        glBindBuffer(GL_ARRAY_BUFFER, circleInstanceVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, circlePositions.size() * sizeof(glm::vec2), circlePositions.data());

        glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, linePoints.size() * sizeof(glm::vec2), linePoints.data());
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Set up 2D orthographic projection
        glm::mat4 projection = glm::ortho(0.0f, (float)SCR_WIDTH, (float)SCR_HEIGHT, 0.0f);

        // Render circles
        if (showCircles) {
            circleShader.use();
            circleShader.setMat4("projection", projection);
            circleShader.setVec3("color", glm::vec3(1.0f, 0.3f, 0.3f));
            circleShader.setFloat("radius", 20.0f);
            circleShader.setFloat("time", currentFrame);

            glBindVertexArray(circleVAO);
            if (wireframe) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            }
            glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 14, circlePositions.size());
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        // Render lines
        if (showLines) {
            lineShader.use();
            lineShader.setMat4("projection", projection);
            lineShader.setVec3("color", glm::vec3(0.3f, 1.0f, 0.3f));
            lineShader.setFloat("time", currentFrame);

            glLineWidth(2.0f);
            glBindVertexArray(lineVAO);
            glDrawArrays(GL_LINES, 0, linePoints.size());
        }

        // ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("2D Renderer Control Panel");
        ImGui::Text("FPS: %.1f", 1.0f / deltaTime);
        ImGui::Separator();
        
        ImGui::Checkbox("Show Circles", &showCircles);
        ImGui::Checkbox("Show Lines", &showLines);
        ImGui::Checkbox("Wireframe", &wireframe);
        
        ImGui::Separator();
        ImGui::Text("Circles: %zu", circlePositions.size());
        ImGui::Text("Lines: %zu", linePoints.size() / 2);
        
        // Add interactive controls
        if (ImGui::Button("Add Circle")) {
            circlePositions.push_back(glm::vec2(
                static_cast<float>(rand() % SCR_WIDTH),
                static_cast<float>(rand() % SCR_HEIGHT)
            ));
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Clear Circles") && !circlePositions.empty()) {
            circlePositions.clear();
        }

        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // glfw: swap buffers and poll IO events
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &circleVAO);
    glDeleteBuffers(1, &circleVBO);
    glDeleteBuffers(1, &circleInstanceVBO);
    glDeleteVertexArrays(1, &lineVAO);
    glDeleteBuffers(1, &lineVBO);

    // ImGui cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}