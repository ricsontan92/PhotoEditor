#include "TextureFilter.h"
#include "GL/glew.h"
#include "FrameBuffer.h"

namespace LibGraphics
{
    const char* VERTEX_SHADER = "               \
        layout(location = 0) in vec3 aPos;      \
        layout(location = 1) in vec2 aTexCoord; \
                                                \
        out vec2 TexCoord;                      \
                                                \
        void main()                             \
        {                                       \
            gl_Position = vec4(aPos, 1.0);      \
            TexCoord = aTexCoord;               \
        }";

	std::shared_ptr<TextureFilter> TextureFilter::CreateFromShader(const std::string& fragShader)
	{
        return CreateFromShaders({ fragShader });
	}

    std::shared_ptr<TextureFilter> TextureFilter::CreateFromShaders(const std::vector<std::string>& fragShaders)
    {
        auto results = std::shared_ptr<TextureFilter>{ new TextureFilter{} };
        for(auto& shader : fragShaders)
        {
            results->shaders.emplace_back(std::move(std::make_shared<Shader>()));
            results->shaders.back()->AddShaderFromString(VERTEX_SHADER, LibGraphics::Shader::TYPE::VERTEX);
            results->shaders.back()->AddShaderFromString(shader, LibGraphics::Shader::TYPE::FRAGMENT);
            if (!results->shaders.back()->GenShaderProgram())
                return nullptr;
        }
        return results;
    }

    std::shared_ptr<Texture> TextureFilter::Apply(const std::shared_ptr<Texture>& texture)
    {
        if(framebuffer == nullptr || framebuffer->GetWidth() != texture->GetWidth() || framebuffer->GetHeight() != texture->GetHeight())
            framebuffer = FrameBuffer::CreateFrameBuffer(texture->GetWidth(), texture->GetHeight());

        auto filteredTexture = texture;
        for (auto& shader : shaders)
        {
            framebuffer->RenderToBuffer([this, &shader, &filteredTexture]() {
                shader->UseProgram();

                for (auto& p : intValues)   shader->SetInt(p.first.c_str(), p.second);
                for (auto& p : floatValues) shader->SetFloat(p.first.c_str(), p.second);
                for (auto& p : vec2Values)  shader->SetVec2(p.first.c_str(), p.second);
                for (auto& p : vec3Values)  shader->SetVec3(p.first.c_str(), p.second);
                for (auto& p : vec4Values)  shader->SetVec4(p.first.c_str(), p.second);

                shader->SetVec2("_Resolution_", LibCore::Math::Vec2{ (float)filteredTexture->GetWidth(), (float)filteredTexture->GetHeight() });

                filteredTexture->Bind();

                glBindVertexArray(quadVAO);
                glDrawArrays(GL_TRIANGLES, 0, 6);
                glBindVertexArray(0);
            });

            filteredTexture = framebuffer->GetGLTexture();
        }

        return framebuffer->GetGLTexture();
    }

    std::shared_ptr<TextureFilter> TextureFilter::Clone() const
    {
        auto results = std::shared_ptr<TextureFilter>{ new TextureFilter{} };
        results->shaders = shaders;
        results->framebuffer = nullptr;
        results->intValues = intValues;
        results->floatValues = floatValues;
        results->vec2Values = vec2Values;
        results->vec3Values = vec3Values;
        results->vec4Values = vec4Values;
        return results;
    }

    void TextureFilter::SetInt(const char* location, int data)
    {
        intValues[location] = data;
    }

    void TextureFilter::SetFloat(const char* location, float data)
    {
        floatValues[location] = data;
    }

    void TextureFilter::SetVec4(const char* location, const LibCore::Math::Vec4& data)
    {
        vec4Values[location] = data;
    }

    void TextureFilter::SetVec3(const char* location, const LibCore::Math::Vec3& data)
    {
        vec3Values[location] = data;
    }

    void TextureFilter::SetVec2(const char* location, const LibCore::Math::Vec2& data)
    {
        vec2Values[location] = data;
    }

    bool TextureFilter::GetInt(const char* location, int& data)
    {
        auto it = intValues.find(location);
        if (it == intValues.end())
            return false;
        data = it->second;
        return true;
    }

    bool TextureFilter::GetFloat(const char* location, float& data)
    {
        auto it = floatValues.find(location);
        if (it == floatValues.end())
            return false;
        data = it->second;
        return true;
    }

    bool TextureFilter::GetVec4(const char* location, LibCore::Math::Vec4& data)
    {
        auto it = vec4Values.find(location);
        if (it == vec4Values.end())
            return false;
        data = it->second;
        return true;
    }

    bool TextureFilter::GetVec3(const char* location, LibCore::Math::Vec3& data)
    {
        auto it = vec3Values.find(location);
        if (it == vec3Values.end())
            return false;
        data = it->second;
        return true;
    }

    bool TextureFilter::GetVec2(const char* location, LibCore::Math::Vec2& data)
    {
        auto it = vec2Values.find(location);
        if (it == vec2Values.end())
            return false;
        data = it->second;
        return true;
    }

	TextureFilter::TextureFilter()
	{
        // Define the quad vertices
        static float quadVertices[] = {
            // Positions        // Texture Coords
            -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,  // Top-left
            -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,  // Bottom-left
             1.0f, -1.0f, 0.0f,  1.0f, 0.0f,  // Bottom-right

            -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,  // Top-left
             1.0f, -1.0f, 0.0f,  1.0f, 0.0f,  // Bottom-right
             1.0f,  1.0f, 0.0f,  1.0f, 1.0f   // Top-right
        };

        // Generate and configure VAO and VBO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);

        glBindVertexArray(quadVAO);

        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

        // Positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        // Texture Coords
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

        glBindVertexArray(0);
	}

    TextureFilter::~TextureFilter()
    {
        glDeleteBuffers(1, &quadVBO);
        glDeleteVertexArrays(1, &quadVAO);
    }
}