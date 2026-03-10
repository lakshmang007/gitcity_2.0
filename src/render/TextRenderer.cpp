#include "TextRenderer.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.h"

TextRenderer::TextRenderer(unsigned int width, unsigned int height) {
    // We'll need a specific shader for text
}

void TextRenderer::load(std::string font, unsigned int fontSize) {
    Characters.clear();
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return;
    }

    FT_Face face;
    if (FT_New_Face(ft, font.c_str(), 0, &face)) {
        std::cerr << "ERROR::FREETYPE: Failed to load font: " << font << std::endl;
        FT_Done_FreeType(ft);
        return;
    }

    FT_Set_Pixel_Sizes(face, 0, fontSize);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); 

    for (unsigned char c = 0; c < 128; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        Character character = {
            texture, 
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            (unsigned int)face->glyph->advance.x
        };
        Characters.insert(std::pair<char, Character>(c, character));
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void TextRenderer::renderText(std::string text, float x, float y, float scale, glm::vec3 color) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Assuming we have a shader active and set up
    // We'll use a local shader object for this
    static Shader textShader("shaders/text.vert", "shaders/text.frag");
    textShader.use();
    textShader.setVec3("textColor", color);
    
    // Set an orthogonal projection matrix for UI text
    glm::mat4 projection = glm::ortho(0.0f, 1280.0f, 0.0f, 720.0f);
    textShader.setMat4("projection", projection);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); ++c) {
        if (Characters.find(*c) == Characters.end()) continue;
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },            
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }           
        };
        
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        x += (ch.Advance >> 6) * scale; 
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);
}

void TextRenderer::renderText3D(std::string text, glm::vec3 worldPos, float scale, glm::vec3 color, const glm::mat4& view, const glm::mat4& projection) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    static Shader textShader("shaders/text.vert", "shaders/text.frag");
    textShader.use();
    textShader.setVec3("textColor", color);
    textShader.setMat4("projection", projection);
    textShader.setMat4("view", view);

    // Billboarding: Calculate right and up vectors in world space from the view matrix
    glm::vec3 CameraRight_worldspace = {view[0][0], view[1][0], view[2][0]};
    glm::vec3 CameraUp_worldspace = {view[0][1], view[1][1], view[2][1]};

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    float xOffset = 0.0f;
    for (char c : text) {
        if (Characters.find(c) == Characters.end()) continue;
        Character ch = Characters[c];

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;

        // Calculate vertex positions relative to worldPos using billboard vectors
        glm::vec3 pos = worldPos + 
                        CameraRight_worldspace * (xOffset + ch.Bearing.x * scale) +
                        CameraUp_worldspace * (float)(ch.Bearing.y - ch.Size.y) * scale;

        float vertices[6][4] = {
            { 0.0f, h, 0.0f, 0.0f },
            { 0.0f, 0.0f, 0.0f, 1.0f },
            { w,    0.0f, 1.0f, 1.0f },
            { 0.0f, h, 0.0f, 0.0f },
            { w,    0.0f, 1.0f, 1.0f },
            { w,    h, 1.0f, 0.0f }
        };

        // Pass the base position to the shader
        textShader.setVec3("worldPos", pos);
        textShader.setVec3("billboardRight", CameraRight_worldspace);
        textShader.setVec3("billboardUp", CameraUp_worldspace);

        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        xOffset += (ch.Advance >> 6) * scale;
    }
    glBindVertexArray(0);
    glDisable(GL_BLEND);
}
