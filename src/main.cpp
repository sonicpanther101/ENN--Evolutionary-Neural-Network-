#include <GL/glew.h>
#include <GLFW/glfw3.h>

// GLM
#include "../vendor/glm/glm/gtc/type_ptr.hpp"
#include "../vendor/glm/glm/gtc/matrix_transform.hpp"
#include "../vendor/glm/glm/gtc/matrix_inverse.hpp"

#include <cstdio>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "shader.h"
#include "camera.h"

#include <iostream>
#include <vector>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// settings
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 800;

bool mouseEnabled = false;
bool CPressed = false;
bool EscPressed = false;
bool wireframe = false;
bool points = false;
bool triangles = true;
bool floorEnabled = true;

// camera
Camera camera(glm::vec3(0.0f, 5.0f, 15.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "ENN", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSwapInterval(0);

    // glew: load all OpenGL function pointers
    // ---------------------------------------
    GLenum err = glewInit();
	if (err != GLEW_OK) {
		std::cout << "Failed to initialize GLEW: %s" << glewGetErrorString(err) << std::endl;
        return -1;
	}

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // render 1 frame to stop flashbang startup
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glfwSwapBuffers(window);
    glfwPollEvents();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    std::vector<glm::vec3> positions = {
        glm::vec3(-0.5f, 0.0f, -0.5f),
        glm::vec3( 0.0f, 1.0f,  0.0f),
        glm::vec3( 0.5f, 0.0f, -0.5f)
    };

    // build and compile our shader program
    // ------------------------------------
    Shader circleShader("../shaders/point_vertex.glsl", "../shaders/point_fragment.glsl");
    Shader lineShader("../shaders/line_vertex.glsl", "../shaders/line_fragment.glsl");

    circleShader.use();
    circleShader.setVec3("color", glm::vec3(1.0f, 0.0f, 0.0f));

    unsigned int billboardVAO, billboardVBO, circlesVBO, indexVBO;
    const float quadVertices[] = {
        // Positions
        -0.5f,  0.5f,
        -0.5f, -0.5f,
        0.5f, -0.5f,
        -0.5f,  0.5f,
        0.5f, -0.5f,
        0.5f,  0.5f
    };

    glGenVertexArrays(1, &billboardVAO);
    glGenBuffers(1, &billboardVBO);
    glGenBuffers(1, &circlesVBO);
    glGenBuffers(1, &indexVBO);

    glBindVertexArray(billboardVAO);

    // Vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, billboardVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    // Vertex attributes
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    // Instance buffer
    glBindBuffer(GL_ARRAY_BUFFER, circlesVBO);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glVertexAttribDivisor(1, 1);  // Update once per instance

    glBindVertexArray(0);

    // rendering cube lines
    std::vector<unsigned int> lineIndices;
    unsigned int linesVAO, linesVBO, linesEBO;
    glGenVertexArrays(1, &linesVAO);
    glGenBuffers(1, &linesVBO);
    glGenBuffers(1, &linesEBO);

    glBindVertexArray(linesVAO);
    glBindBuffer(GL_ARRAY_BUFFER, linesVBO);
    glBufferData(GL_ARRAY_BUFFER, lineIndices.size() * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, linesEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, lineIndices.size() * sizeof(unsigned int), lineIndices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glBindVertexArray(0);

    // Imgui stuff
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // #if defined(__linux__)
    //     io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // #else
    //     io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;
    // #endif

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 450 core");

    // render loop
    // -----------

    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // physicsSystem.GetParticlePositions(positions);

        // Update instance data
        glBindBuffer(GL_ARRAY_BUFFER, circlesVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, positions.size() * sizeof(glm::vec3), positions.data());
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // render
        // ------
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // render the scene

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        circleShader.setMat4("projection", projection);
        circleShader.setMat4("view", view);

        circleShader.use();

        circleShader.setVec3("viewPos", camera.Position);
        circleShader.setFloat("time", static_cast<float>(glfwGetTime()));

        // Billboard-specific uniforms
        circleShader.setVec3("cameraRight", camera.Right);
        circleShader.setVec3("cameraUp", camera.Up);

        // Draw billboards
        glBindVertexArray(billboardVAO);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, positions.size());

        // Lines
        glLineWidth(4.0f); // Make lines thicker
        lineShader.use();
        lineShader.setMat4("projection", projection);
        lineShader.setMat4("view", view);
        glBindVertexArray(linesVAO);
        glDrawElements(GL_LINES, lineIndices.size(), GL_UNSIGNED_INT, 0);

        // ImGui
        ImGui::Begin("Control Panel");
        ImGui::Text("FPS: %.1f", 1.0f / deltaTime);
		ImGui::End();

        ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // // Update and Render additional Platform Windows
        // if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        //     GLFWwindow* backup_current_context = glfwGetCurrentContext();
        //     ImGui::UpdatePlatformWindows();
        //     ImGui::RenderPlatformWindowsDefault();
        //     glfwMakeContextCurrent(backup_current_context);
        // }

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}