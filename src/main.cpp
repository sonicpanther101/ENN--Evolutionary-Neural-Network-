#include "renderable.h"
#include "physics.h"
#include "window.h"
#include "renderer2d.h"
#include "shader.h"


int main() {
    // make the window
    Window window(800, 600, "2D Renderer");

    // setup the renderer
    Shader shader("../shaders/vertex.glsl", "../shaders/fragment.glsl");
    Renderer2D renderer(window.getWidth(), window.getHeight(), shader);

    // Make a circle object
    RenderableObject ball = RenderableObject::makeCircle({200,200}, 40, {1,0,0}, true);

    // Attach a physics body
    PhysicsBody body;
    body.velocity = {50.0f, 30.0f}; // pixels/sec
    body.acceleration = {0.0f, -9.8f}; // simple gravity

    while (!window.shouldClose()) {
        float dt = 0.016f; // fake ~60fps, replace with glfwGetTime()

        // physics update
        body.update(ball, dt);

        glClear(GL_COLOR_BUFFER_BIT);

        renderer.begin();
        renderer.drawObject(ball);
        renderer.end();

        window.swapBuffers();
        window.pollEvents();
    }

    return 0;
}