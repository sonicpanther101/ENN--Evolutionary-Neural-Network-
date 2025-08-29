#pragma once
#include <GL/glew.h>
#include <vector>
#include <cmath>
#include "../vendor/glm/glm/gtc/type_ptr.hpp"
#include "../vendor/glm/glm/gtc/matrix_transform.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// GPU-aligned struct (std430 layout)
struct GPUPhysicsObject {
    glm::vec4 position;     // 16 bytes
    glm::vec4 velocity;     // 16 bytes
    glm::vec4 acceleration; // 16 bytes
    float mass;             // 4 bytes
};

class GPUPhysicsSystem {
public:
    GPUPhysicsSystem(int max_objects = 1000, int iterations = 1);
    ~GPUPhysicsSystem();
    
    void addObject(const GPUPhysicsObject& obj);
    void update(float dt);
    void setIterations(int iterations);
    std::vector<GPUPhysicsObject> getObjectsData();
    
    GLuint getDataBuffer() const { return data_buffer; }
    int getObjectCount() const { return object_count; }

private:
    GLuint compute_shader_program;
    GLuint data_buffer;
    
    int max_objects;
    int iterations;
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