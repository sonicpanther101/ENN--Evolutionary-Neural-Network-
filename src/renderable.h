#pragma once
#include "../vendor/glm/glm/gtc/type_ptr.hpp" 
#include <string>

enum class ShapeType {
    Line,
    Circle,
    Polygon
};

struct RenderableObject {
    ShapeType type;
    glm::vec2 position;      // for center or start point
    glm::vec2 position2;     // for end point (line only)
    float radius = 0.0f;     // for circles/polygons
    int sides = 0;           // polygon sides
    glm::vec3 color {1.0f, 1.0f, 1.0f};
    bool filled = false;

    // convenient constructor helpers
    static RenderableObject makeLine(glm::vec2 p1, glm::vec2 p2, glm::vec3 c) {
        RenderableObject obj;
        obj.type = ShapeType::Line;
        obj.position = p1;
        obj.position2 = p2;
        obj.color = c;
        return obj;
    }

    static RenderableObject makeCircle(glm::vec2 center, float r, glm::vec3 c, bool fill=false) {
        RenderableObject obj;
        obj.type = ShapeType::Circle;
        obj.position = center;
        obj.radius = r;
        obj.color = c;
        obj.filled = fill;
        return obj;
    }

    static RenderableObject makePolygon(glm::vec2 center, float r, int n, glm::vec3 c, bool fill=false) {
        RenderableObject obj;
        obj.type = ShapeType::Polygon;
        obj.position = center;
        obj.radius = r;
        obj.sides = n;
        obj.color = c;
        obj.filled = fill;
        return obj;
    }
};
