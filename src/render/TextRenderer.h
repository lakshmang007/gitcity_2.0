#ifndef TEXTRENDERER_H
#define TEXTRENDERER_H

#include <map>
#include <string>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

struct Character {
    unsigned int TextureID; 
    glm::ivec2   Size;      
    glm::ivec2   Bearing;   
    unsigned int Advance;   
};

class TextRenderer {
public:
    std::map<char, Character> Characters;
    unsigned int VAO, VBO;

    TextRenderer(unsigned int width, unsigned int height);
    void load(std::string font, unsigned int fontSize);
    void renderText(std::string text, float x, float y, float scale, glm::vec3 color);
    void renderText3D(std::string text, glm::vec3 worldPos, float scale, glm::vec3 color, const glm::mat4& view, const glm::mat4& projection);
};

#endif
