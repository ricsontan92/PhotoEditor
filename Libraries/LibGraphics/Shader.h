#pragma once

#include <string>
#include <unordered_map>

#include "LibCore/Vec2.h"
#include "LibCore/Vec3.h"
#include "LibCore/Vec4.h"
#include "LibCore/Mat4.h"

namespace LibGraphics
{
	class Shader
	{
	public:
		enum class TYPE
		{
			VERTEX = 0,
			FRAGMENT
		};

		enum class UTYPE
		{
			FLOAT = 0,
			FLOAT_VEC2,
			FLOAT_VEC3,
			FLOAT_VEC4,
			FLOAT_MAT4,

			INT,
			INT_VEC2,
			INT_VEC3,
			INT_VEC4,

			UINT,
			UINT_VEC2,
			UINT_VEC3,
			UINT_VEC4,

			SAMPLER_2D,

			UNKNOWN
		};

		Shader();
		~Shader();

		void AddShaderFromString(const std::string& data, TYPE shaderType);
		void AddShader(const std::string& path, TYPE shaderType);
		void AddDefine(const std::string& define, int value);
		void AddDefine(const std::string& define, float value);
		void AddDefine(const std::string& define, const std::string& value);
		bool GenShaderProgram();
		void UseProgram() const;

		void SetMat44(const char* location, const LibCore::Math::Mat4& data) const;
		void SetMat44(const char* location, unsigned count, const LibCore::Math::Mat4* data) const;

		void SetInt(const char* location, int data) const;
		void SetFloat(const char* location, float data) const;
		void SetVec4(const char* location, const LibCore::Math::Vec4& data) const;
		void SetVec3(const char* location, const LibCore::Math::Vec3& data) const;
		void SetVec2(const char* location, const LibCore::Math::Vec2& data) const;

	private:
		bool CheckShaderProgramLinkStatus(unsigned int program_hdl, std::string& diag_msg) const;
		bool CheckShaderCompileStatus(unsigned int shader_hdl, std::string& diag_msg) const;
		void GetShaderContents(const std::string& shader, std::string& content) const;
		void CompileAllShaders();

		struct ShaderInfo
		{
			std::string shaderContent;
			unsigned int shaderID;
			unsigned int shaderType;
		};

		struct UniformType
		{
			UniformType(int loc, UTYPE type) : uLocation{ loc }, uType{ type } {}
			int uLocation;
			UTYPE uType;
		};

		using ShaderContainer = std::vector<ShaderInfo>;
		using DefineContainer = std::unordered_map<std::string, std::string>;
		ShaderContainer	allShaders;
		DefineContainer	defines;
		unsigned int shaderProgram;
		std::unordered_map<std::string, UniformType> uniformLocs;
	};
}