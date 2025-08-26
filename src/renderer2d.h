#pragma once
#include <GL/glew.h>

// GLM 
#include "../vendor/glm/glm/gtc/type_ptr.hpp" 
#include "../vendor/glm/glm/gtc/matrix_transform.hpp"

#include <vector>
#include "shader.h"
#include "renderable.h"

class Renderer2D {
public:
    Renderer2D(int width, int height, Shader& shader);
    ~Renderer2D();

    void begin();
    void end();

    // Outlined primitives
    void drawLine(glm::vec2 p1, glm::vec2 p2, glm::vec3 color);
    void drawCircle(glm::vec2 center, float radius, glm::vec3 color, int segments = 64);
    void drawPolygon(glm::vec2 center, float radius, int sides, glm::vec3 color);

    // Filled primitives
    void drawFilledCircle(glm::vec2 center, float radius, glm::vec3 color, int segments = 64);
    void drawFilledPolygon(glm::vec2 center, float radius, int sides, glm::vec3 color);

    void drawObject(const RenderableObject& obj);

private:
    GLuint VAO, VBO;
    Shader& shader;
    glm::mat4 projection;
};
