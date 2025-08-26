#include "renderer2d.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Renderer2D::Renderer2D(int width, int height, Shader& shader)
    : shader(shader)
{
    // Orthographic projection matching window size
    projection = glm::ortho(0.0f, float(width), 0.0f, float(height));

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * 1024, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

Renderer2D::~Renderer2D() {
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
}

void Renderer2D::begin() {
    shader.use();
    shader.setMat4("uProjection", projection);
}

void Renderer2D::end() {}

void Renderer2D::drawLine(glm::vec2 p1, glm::vec2 p2, glm::vec3 color) {
    float vertices[4] = { p1.x, p1.y, p2.x, p2.y };

    shader.setVec3("uColor", color);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    glDrawArrays(GL_LINES, 0, 2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Renderer2D::drawCircle(glm::vec2 center, float radius, glm::vec3 color, int segments) {
    std::vector<float> vertices;
    for (int i = 0; i <= segments; i++) {
        float theta = (2.0f * M_PI * i) / segments;
        float x = center.x + radius * cos(theta);
        float y = center.y + radius * sin(theta);
        vertices.push_back(x);
        vertices.push_back(y);
    }

    shader.setVec3("uColor", color);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());

    glDrawArrays(GL_LINE_LOOP, 0, segments);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Renderer2D::drawPolygon(glm::vec2 center, float radius, int sides, glm::vec3 color) {
    std::vector<float> vertices;
    for (int i = 0; i < sides; i++) {
        float theta = (2.0f * M_PI * i) / sides;
        float x = center.x + radius * cos(theta);
        float y = center.y + radius * sin(theta);
        vertices.push_back(x);
        vertices.push_back(y);
    }

    shader.setVec3("uColor", color);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());

    glDrawArrays(GL_LINE_LOOP, 0, sides);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Renderer2D::drawFilledCircle(glm::vec2 center, float radius, glm::vec3 color, int segments) {
    std::vector<float> vertices;
    // center
    vertices.push_back(center.x);
    vertices.push_back(center.y);
    // perimeter
    for (int i = 0; i <= segments; i++) {
        float theta = (2.0f * M_PI * i) / segments;
        float x = center.x + radius * cos(theta);
        float y = center.y + radius * sin(theta);
        vertices.push_back(x);
        vertices.push_back(y);
    }

    shader.setVec3("uColor", color);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());

    glDrawArrays(GL_TRIANGLE_FAN, 0, segments + 2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Renderer2D::drawFilledPolygon(glm::vec2 center, float radius, int sides, glm::vec3 color) {
    std::vector<float> vertices;
    // center
    vertices.push_back(center.x);
    vertices.push_back(center.y);
    // perimeter
    for (int i = 0; i <= sides; i++) {
        float theta = (2.0f * M_PI * i) / sides;
        float x = center.x + radius * cos(theta);
        float y = center.y + radius * sin(theta);
        vertices.push_back(x);
        vertices.push_back(y);
    }

    shader.setVec3("uColor", color);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());

    glDrawArrays(GL_TRIANGLE_FAN, 0, sides + 2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Renderer2D::drawObject(const RenderableObject& obj) {
    switch (obj.type) {
        case ShapeType::Line:
            drawLine(obj.position, obj.position2, obj.color);
            break;
        case ShapeType::Circle:
            if (obj.filled)
                drawFilledCircle(obj.position, obj.radius, obj.color);
            else
                drawCircle(obj.position, obj.radius, obj.color);
            break;
        case ShapeType::Polygon:
            if (obj.filled)
                drawFilledPolygon(obj.position, obj.radius, obj.sides, obj.color);
            else
                drawPolygon(obj.position, obj.radius, obj.sides, obj.color);
            break;
    }
}