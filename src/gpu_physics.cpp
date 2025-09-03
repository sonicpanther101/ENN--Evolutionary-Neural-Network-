#include "gpu_physics.h"
#include <iostream>
#include <fstream>

// Load the shader file
std::ifstream shaderFile("../shaders/compute_shader.glsl");
std::string compute_shader_source((std::istreambuf_iterator<char>(shaderFile)), std::istreambuf_iterator<char>());
const char* compute_source = compute_shader_source.c_str();

GPUPhysicsSystem::GPUPhysicsSystem(int max_objects, int max_constraints, int iterations, int SCREEN_WIDTH, int SCREEN_HEIGHT) 
    : max_objects(max_objects), max_constraints(max_constraints), iterations(iterations), SCREEN_WIDTH(SCREEN_WIDTH), SCREEN_HEIGHT(SCREEN_HEIGHT), object_count(0), constraint_count(0) {
    
    compute_shader_program = loadComputeShader(compute_source);
    setupBuffers();
}

GPUPhysicsSystem::~GPUPhysicsSystem() {
    glDeleteBuffers(1, &object_data_buffer);
    glDeleteBuffers(1, &constraint_data_buffer);
    glDeleteProgram(compute_shader_program);
}

void GPUPhysicsSystem::setupBuffers() {
    // Single buffer for all object data
    glGenBuffers(1, &object_data_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, object_data_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, max_objects * sizeof(GPUPhysicsObject), nullptr, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &constraint_data_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, constraint_data_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, max_constraints * sizeof(GPUPhysicsConstraint), nullptr, GL_DYNAMIC_DRAW);
}

void GPUPhysicsSystem::addObject(const GPUPhysicsObject& obj) {
    if (object_count >= max_objects) return;
    
    // Upload entire object to buffer
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, object_data_buffer);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 
                    object_count * sizeof(GPUPhysicsObject), 
                    sizeof(GPUPhysicsObject), 
                    &obj);
    
    object_count++;
}

void GPUPhysicsSystem::addConstraint(const GPUPhysicsConstraint& constraint) {
    if (constraint_count >= max_constraints) return;
    
    // Upload entire constraint to buffer
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, constraint_data_buffer);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 
                    constraint_count * sizeof(GPUPhysicsConstraint), 
                    sizeof(GPUPhysicsConstraint), 
                    &constraint);
    
    constraint_count++;
}

void GPUPhysicsSystem::update(float dt) {
    if (object_count == 0) return;
    
    glUseProgram(compute_shader_program);
    
    // Bind single buffer
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, object_data_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, constraint_data_buffer);
    
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
    render_object_program = loadRenderShaders("../shaders/object_vertex_shader.glsl", "../shaders/object_fragment_shader.glsl");
    render_constraint_program = loadRenderShaders("../shaders/constraint_vertex_shader.glsl", "../shaders/constraint_fragment_shader.glsl");
    setupInstancedRendering();
}

GPURenderer2D::~GPURenderer2D() {
    glDeleteProgram(render_object_program);
    glDeleteProgram(render_constraint_program);
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

    glGenVertexArrays(1, &dummy_vao);
    
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

void GPURenderer2D::renderObjects(const GPUPhysicsSystem& physics_system) {
    if (physics_system.getObjectCount() == 0) return;
    
    glUseProgram(render_object_program);
    
    // Set projection matrix
    glUniformMatrix4fv(glGetUniformLocation(render_object_program, "u_projection"), 
                       1, GL_FALSE, glm::value_ptr(projection));
    
    // Bind physics buffer for reading
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, physics_system.getObjectDataBuffer());
    
    // Render all instances
    glBindVertexArray(circle_template_vao);
    
    // Draw as line loops (wireframe) - using all 16 vertices but shader will cull extras
    glDrawArraysInstanced(GL_LINE_LOOP, 0, circle_segments, physics_system.getObjectCount());
    
    glBindVertexArray(0);
}

void GPURenderer2D::renderConstraints(const GPUPhysicsSystem& physics_system) {
    if (physics_system.getConstraintCount() == 0) return;

    glUseProgram(render_constraint_program);

    glUniformMatrix4fv(glGetUniformLocation(render_constraint_program, "u_projection"), 
                       1, GL_FALSE, glm::value_ptr(projection));

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, physics_system.getObjectDataBuffer());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, physics_system.getConstraintDataBuffer());

    // Draw each constraint as a line (2 vertices per instance)
    glBindVertexArray(dummy_vao);
    glDrawArraysInstanced(GL_LINES, 0, 2, physics_system.getConstraintCount());
    glBindVertexArray(0);
}

GLuint GPURenderer2D::loadRenderShaders(const char* instanced_vertex_shader_path, const char* instanced_fragment_shader_path) {

    // Load the shader file
    std::ifstream shaderFile(instanced_vertex_shader_path);
    std::string shader_source((std::istreambuf_iterator<char>(shaderFile)), std::istreambuf_iterator<char>());
    const char* instanced_vertex_shader = shader_source.c_str();

    // Load the shader file
    std::ifstream shaderFile2(instanced_fragment_shader_path);
    std::string shader_source2((std::istreambuf_iterator<char>(shaderFile2)), std::istreambuf_iterator<char>());
    const char* instanced_fragment_shader = shader_source2.c_str();

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
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, object_data_buffer);
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