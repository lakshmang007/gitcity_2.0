#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;

uniform mat4 projection;
uniform mat4 view;
uniform vec3 worldPos;
uniform vec3 billboardRight;
uniform vec3 billboardUp;

void main() {
    // Basic billboarding in vertex shader
    vec3 pos = worldPos + 
               billboardRight * vertex.x + 
               billboardUp * vertex.y;
               
    gl_Position = projection * view * vec4(pos, 1.0);
    TexCoords = vertex.zw;
}
