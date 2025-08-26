#include "../vendor/glm/glm/gtc/type_ptr.hpp" 
#include "renderable.h"

struct PhysicsBody {
    glm::vec2 velocity {0.0f, 0.0f};
    glm::vec2 acceleration {0.0f, 0.0f};

    void update(RenderableObject& obj, float dt) {
        velocity += acceleration * dt;
        obj.position += velocity * dt;
        // if line, also move second point
        if (obj.type == ShapeType::Line) {
            obj.position2 += velocity * dt;
        }
    }
};