#include "Shader.h"
#include "GL/glew.h"

#include <cassert>
#include <iostream>

namespace LibGraphics
{
	Shader::Shader() : shaderProgram(0)
	{
	}

	void Shader::AddShader(const std::string& path, TYPE shaderType)
	{
		std::string content;
		GetShaderContents(path.c_str(), content);
		if (content.empty())
			throw std::exception("Shader file not found.");
		AddShaderFromString(content, shaderType);
	}

	void Shader::AddShaderFromString(const std::string& data, TYPE shaderType)
	{
		ShaderInfo info;
		info.shaderContent = data;

		if (info.shaderContent.empty())
			throw std::exception("Shader file not found.");

		switch (shaderType)
		{
		case TYPE::VERTEX: info.shaderType = GL_VERTEX_SHADER; break;
		case TYPE::FRAGMENT: info.shaderType = GL_FRAGMENT_SHADER; break;
		default:
			assert(false); // shader type not implemented
		}
		assert(shaderProgram == 0); // Shader already created

		allShaders.emplace_back(std::move(info));
	}

	void Shader::AddDefine(const std::string& define, int value)
	{
		AddDefine(define, std::to_string(value));
	}

	void Shader::AddDefine(const std::string& define, float value)
	{
		AddDefine(define, std::to_string(value));
	}

	void Shader::AddDefine(const std::string& define, const std::string& value)
	{
		assert(shaderProgram == 0); // Shader already created
		defines[define] = value;
	}

	bool Shader::CheckShaderCompileStatus(unsigned int shader_hdl, std::string& diag_msg) const
	{
		GLint result;
		glGetShaderiv(shader_hdl, GL_COMPILE_STATUS, &result);
		if (GL_FALSE == result)
		{
			GLint log_len;
			glGetShaderiv(shader_hdl, GL_INFO_LOG_LENGTH, &log_len);
			if (log_len > 0)
			{
				char* error_log_str = new GLchar[log_len];
				GLsizei written_log_len;
				glGetShaderInfoLog(shader_hdl, log_len, &written_log_len, error_log_str);
				diag_msg = error_log_str;
				delete[] error_log_str;
			}
			return false;
		}
		return true;
	}

	bool Shader::CheckShaderProgramLinkStatus(unsigned int program_hdl, std::string& diag_msg) const
	{
		GLint result;
		glGetProgramiv(program_hdl, GL_LINK_STATUS, &result);
		if (GL_FALSE == result)
		{
			GLint log_len;
			glGetProgramiv(program_hdl, GL_INFO_LOG_LENGTH, &log_len);
			if (log_len > 0)
			{
				char* error_log_str = new GLchar[log_len];
				GLsizei written_log_len;
				glGetProgramInfoLog(program_hdl, log_len, &written_log_len, error_log_str);
				diag_msg = error_log_str;
				delete[] error_log_str;
			}
			return false;
		}
		return true;
	}

	void Shader::GetShaderContents(const std::string& shader, std::string& content) const
	{
		FILE* file;
		size_t count = 0;

		if (shader.c_str() != NULL)
		{
			fopen_s(&file, shader.c_str(), "rt");
			if (file != NULL)
			{
				fseek(file, 0, SEEK_END);
				count = ftell(file);
				rewind(file);

				if (count > 0)
				{
					char* bufcontent = (char*)malloc(sizeof(char) * (count + 1));
					count = fread(bufcontent, sizeof(char), count, file);
					bufcontent[count] = '\0';
					content = bufcontent;
					free(bufcontent);
				}

				fclose(file);
			}
			else
			{
				std::cerr << shader << " not found." << std::endl;
			}
		}
	}

	bool Shader::GenShaderProgram()
	{
		shaderProgram = glCreateProgram();				// generate the prog

		CompileAllShaders();

		for (auto& elem : allShaders)
			glAttachShader(shaderProgram, elem.shaderID);	// attach the shaders

		glLinkProgram(shaderProgram);	// link the shader
		std::string errMsg;
		if (!CheckShaderProgramLinkStatus(shaderProgram, errMsg))
		{
			std::cerr << errMsg << std::endl;
			return false;
		}

		glValidateProgram(shaderProgram);	// validate the shader

		// clear unused shader
		for (auto& elem : allShaders)
		{
			glDetachShader(shaderProgram, elem.shaderID);
			glDeleteShader(elem.shaderID);
		}

		allShaders.clear();

		{
			glUseProgram(shaderProgram);
			int count, size;
			GLenum type;
			char name[64];
			GLsizei length;
			glGetProgramiv(shaderProgram, GL_ACTIVE_UNIFORMS, &count);
			for (int i = 0; i < count; i++)
			{
				glGetActiveUniform(shaderProgram, (GLuint)i, sizeof(name), &length, &size, &type, name);
				
				UTYPE uniformType = UTYPE::UNKNOWN;
				switch (type)
				{
				case GL_FLOAT: uniformType = UTYPE::FLOAT; break;
				case GL_FLOAT_VEC2: uniformType = UTYPE::FLOAT_VEC2; break;
				case GL_FLOAT_VEC3: uniformType = UTYPE::FLOAT_VEC3; break;
				case GL_FLOAT_VEC4: uniformType = UTYPE::FLOAT_VEC4; break;
				case GL_FLOAT_MAT4: uniformType = UTYPE::FLOAT_MAT4; break;

				case GL_INT: uniformType = UTYPE::INT; break;
				case GL_INT_VEC2: uniformType = UTYPE::INT_VEC2; break;
				case GL_INT_VEC3: uniformType = UTYPE::INT_VEC3; break;
				case GL_INT_VEC4: uniformType = UTYPE::INT_VEC4; break;

				case GL_UNSIGNED_INT: uniformType = UTYPE::UINT; break;
				case GL_UNSIGNED_INT_VEC2: uniformType = UTYPE::UINT_VEC2; break;
				case GL_UNSIGNED_INT_VEC3: uniformType = UTYPE::UINT_VEC3; break;
				case GL_UNSIGNED_INT_VEC4: uniformType = UTYPE::UINT_VEC4; break;

				case GL_SAMPLER_2D: uniformType = UTYPE::SAMPLER_2D; break;

				default: break;
				};
				uniformLocs.insert(std::pair<std::string, UniformType>(name, { i, uniformType }));
			}
		}

		return true;
	}

	void Shader::CompileAllShaders()
	{
		std::string errMsg;

		for (auto& shader : allShaders)
		{
			if (shader.shaderContent.empty())
				continue;

			GLuint& retID = shader.shaderID;				// get id
			retID = glCreateShader(shader.shaderType);		// create shader
			std::string definesStr;
			for (auto& elem : defines)
				definesStr += "#define " + elem.first + " (" + elem.second + ")\n";

			shader.shaderContent = "#version 430\n" + definesStr + shader.shaderContent;
			const char* pBuffer = shader.shaderContent.c_str();
			glShaderSource(retID, 1, &pBuffer, nullptr);	// put source into memory
			glCompileShader(retID);							// compile

			if (!CheckShaderCompileStatus(retID, errMsg))
			{
				std::cerr << errMsg << std::endl;
				return;
			}
		}
	}

	void Shader::UseProgram() const
	{
		glUseProgram(shaderProgram);
	}

	void Shader::SetMat44(const char* location, const LibCore::Math::Mat4& data) const
	{
		auto it = uniformLocs.find(location);
		if (it != uniformLocs.end())
			glUniformMatrix4fv(it->second.uLocation, 1, GL_FALSE, data.mat);
	}

	void Shader::SetMat44(const char* location, unsigned count, const LibCore::Math::Mat4* data) const
	{
		auto it = uniformLocs.find(location);
		if (it != uniformLocs.end())
			glUniformMatrix4fv(it->second.uLocation, count, GL_FALSE, (*data).mat);
	}

	void Shader::SetInt(const char* location, int data) const
	{
		auto it = uniformLocs.find(location);
		if (it != uniformLocs.end())
			glUniform1i(it->second.uLocation, data);
	}

	void Shader::SetFloat(const char* location, float data) const
	{
		auto it = uniformLocs.find(location);
		if (it != uniformLocs.end())
			glUniform1f(it->second.uLocation, data);
	}

	void Shader::SetVec4(const char* location, const LibCore::Math::Vec4& data) const
	{
		auto it = uniformLocs.find(location);
		if (it != uniformLocs.end())
			glUniform4fv(it->second.uLocation, 1, &data.x);
	}

	void Shader::SetVec3(const char* location, const LibCore::Math::Vec3& data) const
	{
		auto it = uniformLocs.find(location);
		if (it != uniformLocs.end())
			glUniform3fv(it->second.uLocation, 1, &data.x);
	}

	void Shader::SetVec2(const char* location, const LibCore::Math::Vec2& data) const
	{
		auto it = uniformLocs.find(location);
		if (it != uniformLocs.end())
			glUniform2fv(it->second.uLocation, 1, &data.x);
	}

	Shader::~Shader()
	{
		glDeleteProgram(shaderProgram);
		glUseProgram(0);
	}
}