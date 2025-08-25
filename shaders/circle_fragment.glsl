#version 450 core

in vec2 fragPos;
in vec2 center;

uniform vec3 color;
uniform float radius;
uniform float time;

out vec4 FragColor;

void main() {
    float dist = length(fragPos - center);
    
    // Create smooth circle edge
    float alpha = 1.0 - smoothstep(radius - 1.0, radius, dist);
    
    // Optional: Add some animation based on time
    vec3 animatedColor = color * (0.8 + 0.2 * sin(time * 2.0));
    
    FragColor = vec4(animatedColor, alpha);
}