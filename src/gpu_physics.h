// src/gpu_physics.h
#pragma once
#include <GL/glew.h>
#include <vector>
#include <cmath>
#include "../vendor/glm/glm/gtc/type_ptr.hpp"
#include "../vendor/glm/glm/gtc/matrix_transform.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

struct GPUPhysicsObject {
    glm::vec2 position;
    glm::vec2 velocity;
    glm::vec2 acceleration;
    float radius;
    glm::vec3 color;
    int shape_type; // 0=line, 1=circle, 2=polygon
    int sides;
    glm::vec2 position2; // for lines
    int filled;
    float padding[3]; // align to 64 bytes
};

class GPUPhysicsSystem {
public:
    GPUPhysicsSystem(int max_objects = 1000);
    ~GPUPhysicsSystem();
    
    void addObject(const GPUPhysicsObject& obj);
    void update(float dt);
    
    GLuint getDataBuffer() const { return data_buffer; }
    int getObjectCount() const { return object_count; }

private:
    GLuint compute_shader_program;
    GLuint data_buffer;
    
    int max_objects;
    int object_count;
    
    void setupBuffers();
    GLuint loadComputeShader(const char* compute_source);
};

class GPURenderer2D {
public:
    GPURenderer2D(int width, int height);
    ~GPURenderer2D();
    
    void render(const GPUPhysicsSystem& physics_system);

private:
    GLuint render_program;
    GLuint circle_template_vao, circle_template_vbo;
    glm::mat4 projection;
    
    void setupInstancedRendering();
    GLuint loadRenderShaders();
};