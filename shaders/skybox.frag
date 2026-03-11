#version 330 core
out vec4 FragColor;

in vec3 FragPos;

void main() {
    // Procedural sky gradient
    float y = normalize(FragPos).y;
    
    // Sky: blend from horizon haze to deep blue at top
    vec3 topColor    = vec3(0.20, 0.40, 0.80);  // Deep blue
    vec3 horizColor  = vec3(0.55, 0.70, 0.85);  // Hazy light blue
    vec3 bottomColor = vec3(0.10, 0.12, 0.18);  // Dark below horizon
    
    vec3 color;
    if (y > 0.0) {
        color = mix(horizColor, topColor, pow(y, 0.6));
    } else {
        color = mix(horizColor, bottomColor, pow(-y, 0.4));
    }
    
    // Subtle sun glow
    vec3 sunDir = normalize(vec3(0.4, 0.8, 0.3));
    float sunDot = max(dot(normalize(FragPos), sunDir), 0.0);
    color += vec3(1.0, 0.9, 0.6) * pow(sunDot, 64.0) * 0.4;  // Sun disk
    color += vec3(1.0, 0.8, 0.5) * pow(sunDot, 8.0)  * 0.1;  // Sun halo
    
    FragColor = vec4(color, 1.0);
}
