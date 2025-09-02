#include "gpu_physics.h"
#include <iostream>
#include <fstream>

// Load the shader file
std::ifstream shaderFile("../shaders/compute_shader.glsl");
std::string compute_shader_source((std::istreambuf_iterator<char>(shaderFile)), std::istreambuf_iterator<char>());
const char* compute_source = compute_shader_source.c_str();


const char* instanced_vertex_shader = R"(
#version 430 core

layout(location = 0) in vec2 template_pos;

struct PhysicsObject {
    vec4 position;
    vec4 velocity;
    vec4 acceleration;  
    float mass;
    float radius;
};

layout(std430, binding = 0) restrict readonly buffer ObjectBuffer {
    PhysicsObject objects[];
};

uniform mat4 u_projection;

out vec3 v_color;

void main() {
    PhysicsObject obj = objects[gl_InstanceID];
    
    vec2 world_pos;
    
    // circle
    world_pos = obj.position.xy + template_pos * obj.radius;
    
    gl_Position = u_projection * vec4(world_pos, 0.0, 1.0);
    v_color = vec3(1.0);
}
)";

const char* instanced_fragment_shader = R"(
#version 430 core

in vec3 v_color;
out vec4 fragment_color;

void main() {
    fragment_color = vec4(v_color, 1.0);
}
)";

GPUPhysicsSystem::GPUPhysicsSystem(int max_objects, int iterations, int SCREEN_WIDTH, int SCREEN_HEIGHT) 
    : max_objects(max_objects), iterations(iterations), SCREEN_WIDTH(SCREEN_WIDTH), SCREEN_HEIGHT(SCREEN_HEIGHT), object_count(0) {
    
    compute_shader_program = loadComputeShader(compute_source);
    setupBuffers();
}

GPUPhysicsSystem::~GPUPhysicsSystem() {
    glDeleteBuffers(1, &data_buffer);
    glDeleteProgram(compute_shader_program);
}

void GPUPhysicsSystem::setupBuffers() {
    // Single buffer for all object data
    glGenBuffers(1, &data_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, data_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, max_objects * sizeof(GPUPhysicsObject), nullptr, GL_DYNAMIC_DRAW);
}

void GPUPhysicsSystem::addObject(const GPUPhysicsObject& obj) {
    if (object_count >= max_objects) return;
    
    // Upload entire object to buffer
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, data_buffer);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 
                    object_count * sizeof(GPUPhysicsObject), 
                    sizeof(GPUPhysicsObject), 
                    &obj);
    
    object_count++;
}

void GPUPhysicsSystem::update(float dt) {
    if (object_count == 0) return;
    
    glUseProgram(compute_shader_program);
    
    // Bind single buffer
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, data_buffer);
    
    // Set uniforms
    glUniform1f(glGetUniformLocation(compute_shader_program, "u_deltaTime"), dt);
    glUniform2f(glGetUniformLocation(compute_shader_program, "u_screenSize"), SCREEN_WIDTH, SCREEN_HEIGHT);
    glUniform1i(glGetUniformLocation(compute_shader_program, "u_iterations"), iterations);
    
    // Dispatch compute shader
    int work_groups = (object_count + 63) / 64;
    glDispatchCompute(work_groups, 1, 1);
    
    // Memory barrier
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

GLuint GPUPhysicsSystem::loadComputeShader(const char* compute_source) {
    GLuint compute_shader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(compute_shader, 1, &compute_source, nullptr);
    glCompileShader(compute_shader);
    
    // Check compilation
    int success;
    char info_log[512];
    glGetShaderiv(compute_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(compute_shader, 512, nullptr, info_log);
        std::cerr << "Compute shader compilation failed: " << info_log << std::endl;
    }
    
    GLuint program = glCreateProgram();
    glAttachShader(program, compute_shader);
    glLinkProgram(program);
    
    // Check linking
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, nullptr, info_log);
        std::cerr << "Compute shader program linking failed: " << info_log << std::endl;
    }
    
    glDeleteShader(compute_shader);
    return program;
}

// GPURenderer2D Implementation
GPURenderer2D::GPURenderer2D(int width, int height) {
    projection = glm::ortho(0.0f, float(width), 0.0f, float(height));
    render_program = loadRenderShaders();
    setupInstancedRendering();
}

GPURenderer2D::~GPURenderer2D() {
    glDeleteProgram(render_program);
    glDeleteVertexArrays(1, &circle_template_vao);
    glDeleteBuffers(1, &circle_template_vbo);
}

void GPURenderer2D::setupInstancedRendering() {
    // Create a simple vertex template that we'll use for all shapes
    // We'll generate actual geometry in the vertex shader
    std::vector<float> vertices;
    
    // Create enough vertices to handle the largest polygon (let's say up to 16 sides)
    for (int i = 0; i < circle_segments; ++i) {
        float angle = 2.0f * M_PI * i / circle_segments;
        vertices.push_back(cosf(angle));
        vertices.push_back(sinf(angle));
    }
    
    glGenVertexArrays(1, &circle_template_vao);
    glGenBuffers(1, &circle_template_vbo);
    
    glBindVertexArray(circle_template_vao);
    glBindBuffer(GL_ARRAY_BUFFER, circle_template_vbo);
    glBufferData(GL_ARRAY_BUFFER, 
                 vertices.size() * sizeof(float), 
                 vertices.data(), 
                 GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    
    glBindVertexArray(0);
}

void GPURenderer2D::render(const GPUPhysicsSystem& physics_system) {
    if (physics_system.getObjectCount() == 0) return;
    
    glUseProgram(render_program);
    
    // Set projection matrix
    glUniformMatrix4fv(glGetUniformLocation(render_program, "u_projection"), 
                       1, GL_FALSE, glm::value_ptr(projection));
    
    // Bind physics buffer for reading
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, physics_system.getDataBuffer());
    
    // Render all instances
    glBindVertexArray(circle_template_vao);
    
    // Draw as line loops (wireframe) - using all 16 vertices but shader will cull extras
    glDrawArraysInstanced(GL_LINE_LOOP, 0, circle_segments, physics_system.getObjectCount());
    
    glBindVertexArray(0);
}

GLuint GPURenderer2D::loadRenderShaders() {
    // Compile vertex shader
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &instanced_vertex_shader, nullptr);
    glCompileShader(vertex_shader);
    
    // Check vertex shader compilation
    int success;
    char info_log[512];
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex_shader, 512, nullptr, info_log);
        std::cerr << "Vertex shader compilation failed: " << info_log << std::endl;
    }
    
    // Compile fragment shader
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &instanced_fragment_shader, nullptr);
    glCompileShader(fragment_shader);
    
    // Check fragment shader compilation
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment_shader, 512, nullptr, info_log);
        std::cerr << "Fragment shader compilation failed: " << info_log << std::endl;
    }
    
    // Create program
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    
    // Check program linking
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, nullptr, info_log);
        std::cerr << "Shader program linking failed: " << info_log << std::endl;
    }
    
    // Cleanup
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    
    return program;
}

std::vector<GPUPhysicsObject> GPUPhysicsSystem::getObjectsData() {
    std::vector<GPUPhysicsObject> data(object_count);
    
    // Bind and read from GPU buffer
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, data_buffer);
    GLvoid* ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
    if (ptr) {
        memcpy(data.data(), ptr, object_count * sizeof(GPUPhysicsObject));
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    
    return data;
}

void GPUPhysicsSystem::setIterations(int iterations) {
    this->iterations = iterations;
}