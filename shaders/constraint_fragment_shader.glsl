#version 430 core

in float v_stress;
out vec4 fragColor;

void main() {
    // Green if relaxed, Red if stretched
    float stress = clamp(abs(v_stress), 0.0, 1.0);
    vec3 color = mix(vec3(0.0, 1.0, 0.0), vec3(1.0, 0.0, 0.0), stress);
    fragColor = vec4(color, 1.0);
}