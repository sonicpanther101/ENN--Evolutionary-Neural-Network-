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
    float radius;           // 4 bytes
    glm::vec2 _pad;        // 8 bytes 
}; // 64 bytes it must be a multiple of 16 bytes

struct GPUPhysicsConstraint {
    int type;
    int indexA;
    int indexB;
    float restLength;
    float stiffness; // k_j^(n)
    float lambda; // λ_j^(n)
    glm::vec2 _pad;
    // Could also add min/max bounds for inequality constraints
}; // 32 bytes

const int circle_segments = 64;

class GPUPhysicsSystem {
public:
    GPUPhysicsSystem(int max_objects = 1000, int max_constraints = 1000, int iterations = 1, int SCREEN_WIDTH = 1600, int SCREEN_HEIGHT = 1200);
    ~GPUPhysicsSystem();
    
    void addObject(const GPUPhysicsObject& obj);
    void addConstraint(const GPUPhysicsConstraint& constraint);
    void update(float dt);
    void setIterations(int iterations);
    std::vector<GPUPhysicsObject> getObjectsData();
    
    GLuint getObjectDataBuffer() const { return object_data_buffer; }
    GLuint getConstraintDataBuffer() const { return constraint_data_buffer; }
    int getObjectCount() const { return object_count; }
    int getConstraintCount() const { return constraint_count; }

private:
    GLuint compute_shader_program;
    GLuint object_data_buffer;
    GLuint constraint_data_buffer;
    
    int max_objects;
    int max_constraints;
    int iterations;
    int object_count;
    int constraint_count;
    int SCREEN_WIDTH, SCREEN_HEIGHT;
    
    void setupBuffers();
    GLuint loadComputeShader(const char* compute_source);
};

class GPURenderer2D {
public:
    GPURenderer2D(int width, int height);
    ~GPURenderer2D();
    
    void renderObjects(const GPUPhysicsSystem& physics_system);
    void renderConstraints(const GPUPhysicsSystem& physics_system);

private:
    GLuint render_object_program;
    GLuint render_constraint_program;
    GLuint circle_template_vao, circle_template_vbo, dummy_vao;
    glm::mat4 projection;
    
    void setupInstancedRendering();
    GLuint loadRenderShaders(const char* instanced_vertex_shader, const char* instanced_fragment_shader);
};