#version 450 core

layout (location = 0) in vec2 aPos;      // Circle vertex position (unit circle)
layout (location = 1) in vec2 aCenter;   // Instance center position

uniform mat4 projection;
uniform float radius;

out vec2 fragPos;
out vec2 center;

void main() {
    // Scale the unit circle by radius and translate to center position
    vec2 worldPos = aCenter + (aPos * radius);
    
    gl_Position = projection * vec4(worldPos, 0.0, 1.0);
    
    fragPos = worldPos;
    center = aCenter;
}