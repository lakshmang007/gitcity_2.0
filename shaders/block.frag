#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
flat in int OutBlockType;

uniform vec3 viewPos;

void main() {
    vec3 color;
    bool isEmissive = false;
    
    if (OutBlockType == 1) color = vec3(0.1, 0.4, 0.1);      // GRASS (darker green)
    else if (OutBlockType == 2) color = vec3(0.5, 0.5, 0.5); // STONE
    else if (OutBlockType == 3) color = vec3(0.4, 0.2, 0.1); // WOOD
    else if (OutBlockType == 4) color = vec3(0.0, 0.3, 0.0); // LEAVES
    else if (OutBlockType == 5) color = vec3(0.0, 0.3, 0.8); // WATER
    else if (OutBlockType == 6) { // WINDOW (Emissive Yellow)
        color = vec3(1.0, 0.9, 0.2); 
        isEmissive = true; 
    }
    else if (OutBlockType == 7) color = vec3(0.6, 0.2, 0.2); // BRICK
    else if (OutBlockType == 8) color = vec3(0.8, 0.9, 1.0); // GLASS
    else if (OutBlockType == 9) color = vec3(0.2, 0.2, 0.2); // METAL/CONCRETE
    else if (OutBlockType == 10) color = vec3(0.1, 0.1, 0.1); // ROAD
    else color = vec3(0.8, 0.8, 0.8);
    
    // Lighting
    vec3 lightPos = vec3(200.0, 500.0, 200.0);
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    
    // Ambient
    float ambientStrength = 0.4;
    vec3 ambient = ambientStrength * lightColor;
    
    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    vec3 result;
    if (isEmissive) {
        // Windows glow regardless of light
        result = color; 
    } else {
        result = (ambient + diffuse) * color;
    }
    
    // Add simple fog
    float distance = length(viewPos - FragPos);
    float fogStrength = clamp(distance / 500.0, 0.0, 1.0);
    vec3 fogColor = vec3(0.05, 0.05, 0.1); // Dark blueish night fog
    
    FragColor = vec4(mix(result, fogColor, fogStrength), 1.0);
}
