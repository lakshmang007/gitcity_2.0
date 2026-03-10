#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

void main() {    
    // Simple procedural night sky
    float dash = sin(TexCoords.x * 100.0) * sin(TexCoords.y * 100.0) * sin(TexCoords.z * 100.0);
    vec3 color = vec3(0.01, 0.01, 0.05); // Very dark blue
    if (dash > 0.99) color = vec3(1.0, 1.0, 1.0); // Simple stars
    
    FragColor = vec4(color, 1.0);
}
