#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
flat in int OutBlockType;

uniform vec3 viewPos;
uniform float time;

void main() {
    vec3 color;
    bool isEmissive = false;
    float alpha = 1.0;
    
    // Block colors matched to enum order: AIR=0, GRASS=1, STONE=2, WOOD=3, LEAVES=4, WATER=5, WINDOW=6, BRICK=7, GLASS=8, METAL=9, ROAD=10
    if      (OutBlockType == 1)  color = vec3(0.18, 0.55, 0.15);     // GRASS
    else if (OutBlockType == 2)  color = vec3(0.55, 0.55, 0.52);     // STONE
    else if (OutBlockType == 3)  color = vec3(0.55, 0.35, 0.18);     // WOOD
    else if (OutBlockType == 4)  color = vec3(0.15, 0.50, 0.10);     // LEAVES
    else if (OutBlockType == 5) {                                     // WATER
        color = vec3(0.10, 0.30, 0.65);
        // Subtle wave shimmer
        color += 0.03 * sin(FragPos.x * 0.5 + FragPos.z * 0.3);
    }
    else if (OutBlockType == 6) {                                     // WINDOW (Emissive)
        color = vec3(1.0, 0.85, 0.3);
        isEmissive = true;
    }
    else if (OutBlockType == 7)  color = vec3(0.62, 0.28, 0.22);     // BRICK
    else if (OutBlockType == 8) {                                     // GLASS
        color = vec3(0.7, 0.85, 0.95);
        alpha = 0.7;
    }
    else if (OutBlockType == 9)  color = vec3(0.35, 0.38, 0.42);     // METAL
    else if (OutBlockType == 10) color = vec3(0.15, 0.15, 0.18);     // ROAD
    else                         color = vec3(0.7, 0.7, 0.7);
    
    // === LIGHTING ===
    vec3 sunDir = normalize(vec3(0.4, 0.8, 0.3));
    vec3 sunColor = vec3(1.0, 0.95, 0.85);
    
    // Ambient
    vec3 ambient = vec3(0.35) * sunColor;
    
    // Diffuse
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, sunDir), 0.0);
    vec3 diffuse = diff * sunColor * 0.7;
    
    // Specular (subtle)
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfDir = normalize(sunDir + viewDir);
    float spec = pow(max(dot(norm, halfDir), 0.0), 32.0);
    vec3 specular = spec * sunColor * 0.15;
    
    vec3 result;
    if (isEmissive) {
        result = color * 1.2; // Glow brighter than surroundings
    } else {
        result = (ambient + diffuse + specular) * color;
    }
    
    // === DISTANCE FOG ===
    float dist = length(viewPos - FragPos);
    float fogStart = 200.0;
    float fogEnd   = 800.0;
    float fogFactor = clamp((dist - fogStart) / (fogEnd - fogStart), 0.0, 0.8);
    vec3 fogColor = vec3(0.55, 0.70, 0.85); // Light blue sky fog
    
    result = mix(result, fogColor, fogFactor);
    
    FragColor = vec4(result, alpha);
}
