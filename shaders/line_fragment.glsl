#version 450 core

uniform vec3 color;
uniform float time;

out vec4 FragColor;

void main() {
    // Optional: Add some animation to lines
    vec3 animatedColor = color * (0.7 + 0.3 * sin(time * 1.5));
    
    FragColor = vec4(animatedColor, 1.0);
}