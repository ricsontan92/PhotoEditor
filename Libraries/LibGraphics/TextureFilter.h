#pragma once

#include <map>
#include <memory>
#include <string>
#include "Shader.h"
#include "Texture.h"
#include "FrameBuffer.h"

namespace LibGraphics
{
	class TextureFilter
	{
	public:
		static std::shared_ptr<TextureFilter> CreateFromShader(const std::string& fragShader);
		static std::shared_ptr<TextureFilter> CreateFromShaders(const std::vector<std::string>& fragShaders);
		std::shared_ptr<Texture> Apply(const std::shared_ptr<Texture>& texture);
		std::shared_ptr<TextureFilter> Clone() const;

		void SetInt(const char* location, int data);
		void SetFloat(const char* location, float data);
		void SetVec4(const char* location, const LibCore::Math::Vec4& data);
		void SetVec3(const char* location, const LibCore::Math::Vec3& data);
		void SetVec2(const char* location, const LibCore::Math::Vec2& data);

		bool GetInt(const char* location, int& data);
		bool GetFloat(const char* location, float& data);
		bool GetVec4(const char* location, LibCore::Math::Vec4& data);
		bool GetVec3(const char* location, LibCore::Math::Vec3& data);
		bool GetVec2(const char* location, LibCore::Math::Vec2& data);

		~TextureFilter();

	private:
		TextureFilter();
		unsigned int quadVAO, quadVBO;
		std::vector<std::shared_ptr<Shader>> shaders;
		std::shared_ptr<FrameBuffer> framebuffer;

		// variables
		std::map<std::string, int> intValues;
		std::map<std::string, float> floatValues;
		std::map<std::string, LibCore::Math::Vec2> vec2Values;
		std::map<std::string, LibCore::Math::Vec3> vec3Values;
		std::map<std::string, LibCore::Math::Vec4> vec4Values;
	};
}