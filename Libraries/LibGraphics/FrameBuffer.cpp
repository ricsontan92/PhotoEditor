#include "FrameBuffer.h"
#include "GL/glew.h"
#include "soil/SOIL.h"

#include <iostream>
#include <filesystem>

namespace LibGraphics
{
	std::shared_ptr<FrameBuffer> FrameBuffer::CreateFrameBuffer(int width, int height)
	{
		return CreateFrameBuffer(width, height, 1);
	}

	std::shared_ptr< FrameBuffer > FrameBuffer::CreateFrameBuffer(int width, int height, int targets)
	{
		width = std::max(1, width);
		height = std::max(1, height);

		auto frameBuffer = std::shared_ptr<FrameBuffer>(new FrameBuffer{ width, height });

		frameBuffer->texture.resize(targets);

		glGenFramebuffers(1, &frameBuffer->fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer->fbo);

		// bind texture
		{
			glGenTextures(targets, frameBuffer->texture.data());

			for (int i = 0; i < targets; ++i)
			{
				glBindTexture(GL_TEXTURE_2D, frameBuffer->texture[i]);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, frameBuffer->texture[i], 0);
			}
		}

		GLenum DrawBuffers[] = {
			GL_COLOR_ATTACHMENT0,
			GL_COLOR_ATTACHMENT1,
			GL_COLOR_ATTACHMENT2,
			GL_COLOR_ATTACHMENT3,
			GL_COLOR_ATTACHMENT4,
			GL_COLOR_ATTACHMENT5,
			GL_COLOR_ATTACHMENT6,
			GL_COLOR_ATTACHMENT7,
			GL_COLOR_ATTACHMENT8,
			GL_COLOR_ATTACHMENT9,
			GL_COLOR_ATTACHMENT10,
			GL_COLOR_ATTACHMENT11,
			GL_COLOR_ATTACHMENT12,
			GL_COLOR_ATTACHMENT13,
			GL_COLOR_ATTACHMENT14,
			GL_COLOR_ATTACHMENT15
		};
		glDrawBuffers(targets, DrawBuffers);

		glGenRenderbuffers(1, &frameBuffer->rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, frameBuffer->rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH32F_STENCIL8, width, height);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, frameBuffer->rbo);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			frameBuffer = nullptr;
			std::cout << "Frame buffer failed to create" << std::endl;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		return frameBuffer;
	}

	void FrameBuffer::Bind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	}

	void FrameBuffer::Unbind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void FrameBuffer::Resize(int width, int height)
	{
		auto copy = CreateFrameBuffer(width, height);
		std::swap(copy->fbo, this->fbo);
		std::swap(copy->height, this->height);
		std::swap(copy->width, this->width);
		std::swap(copy->rbo, this->rbo);
		std::swap(copy->texture, this->texture);
	}

	void FrameBuffer::SetClearColor(const LibCore::Math::Vec4& clearColor)
	{
		this->clearColor = clearColor;
	}

	void FrameBuffer::RenderToBuffer(const std::function<void()>& renderCall) const
	{
		// clear buffers
		Bind();
		{
			glViewport(0, 0, width, height);
			glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			renderCall();
		}
		Unbind();
	}

	std::vector<unsigned char> FrameBuffer::ReadPixels(int target) const
	{
		GLint drawFboId = 0;
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		glReadBuffer(GL_COLOR_ATTACHMENT0 + target);

		std::vector<unsigned char> result(width * height * 3);

		glReadPixels(0, 0, width, height, GL_BGR_EXT, GL_UNSIGNED_BYTE, result.data());

		glBindFramebuffer(GL_FRAMEBUFFER, drawFboId);
		return result;
	}

	std::vector< std::vector<float>> FrameBuffer::ReadMultiTargetPixelsAsFloats(int targetNum) const
	{
		GLint drawFboId = 0;
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		std::vector<std::vector<float>> results;
		auto buffer = std::vector<unsigned char>(width * height * 3);
		for (int i = 0; i < targetNum; ++i)
		{
			glReadBuffer(GL_COLOR_ATTACHMENT0 + i);
			glReadPixels(0, 0, width, height, GL_BGR_EXT, GL_UNSIGNED_BYTE, buffer.data());
			results.emplace_back(std::vector<float>(width * height * 3));
			for (size_t i = 0; i < buffer.size(); ++i)
				results.back()[i] = buffer[i] / 255.0f;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, drawFboId);
		return results;
	}

	std::vector< std::vector<unsigned char>> FrameBuffer::ReadMultiTargetPixels(int targetNum) const
	{
		GLint drawFboId = 0;
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		std::vector<std::vector<unsigned char>> results;
		for (int i = 0; i < targetNum; ++i)
		{
			results.push_back(std::vector<unsigned char>(width * height * 3));
			glReadBuffer(GL_COLOR_ATTACHMENT0 + i);
			glReadPixels(0, 0, width, height, GL_BGR_EXT, GL_UNSIGNED_BYTE, results.back().data());
		}
		glBindFramebuffer(GL_FRAMEBUFFER, drawFboId);
		return results;
	}

	std::vector<float> FrameBuffer::ReadPixelsAsFloats(int target) const
	{
		GLint drawFboId = 0;
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		glReadBuffer(GL_COLOR_ATTACHMENT0 + target);

		std::vector<float> result(width * height * 3);
		glReadPixels(0, 0, width, height, GL_BGR_EXT, GL_FLOAT, result.data());

		glBindFramebuffer(GL_FRAMEBUFFER, drawFboId);

		return result;
	}

	unsigned FrameBuffer::GetGLTextureID(int target) const
	{
		return texture[target];
	}

	std::shared_ptr< Texture > FrameBuffer::GetGLTexture(int target) const
	{
		std::shared_ptr<Texture> result{ new Texture{} };
		result->format = Texture::FORMAT::BGR24;
		result->width = GetWidth();
		result->height = GetHeight();

		glGenTextures(1, &result->texHandler);
		glBindTexture(GL_TEXTURE_2D, result->texHandler);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, result->width, result->height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		// Copy the data from the source texture to the destination texture
		glCopyImageSubData(
			texture[target], GL_TEXTURE_2D, 0, 0, 0, 0,		// Source texture parameters
			result->texHandler, GL_TEXTURE_2D, 0, 0, 0, 0,	// Destination texture parameters
			result->width, result->height, 1				// Dimensions of the copied region
		);
		return result;
	}

	void FrameBuffer::BindTexture(int target) const
	{
		glActiveTexture(GL_TEXTURE0 + target);
		glBindTexture(GL_TEXTURE_2D, GetGLTextureID(0));
	}

	bool FrameBuffer::Save(const std::string& path) const
	{
		const std::string ext = std::filesystem::path{ path }.extension().string();
		int channels = 1;
		std::vector<unsigned char> pixels((size_t)GetWidth() * GetHeight() * channels, 0);
		BindTexture(0);
		glGetTexImage(GL_TEXTURE_2D, 0, channels == 4 ? GL_RGBA : (channels == 1 ? GL_R : GL_RGB), GL_UNSIGNED_BYTE, pixels.data());

		return SOIL_save_image(
			path.c_str(),
			ext == ".bmp" ? SOIL_SAVE_TYPE_BMP : (ext == ".tga" ? SOIL_SAVE_TYPE_TGA : SOIL_SAVE_TYPE_DDS),
			GetWidth(),
			GetHeight(),
			channels,
			pixels.data());
	}

	float FrameBuffer::GetAspect() const
	{
		return width / (float)height;
	}

	int FrameBuffer::GetWidth() const
	{
		return width;
	}

	int FrameBuffer::GetHeight() const
	{
		return height;
	}

	FrameBuffer::~FrameBuffer()
	{
		if (!texture.empty())
			glDeleteTextures((GLsizei)texture.size(), texture.data());
		if (rbo)
			glDeleteRenderbuffers(1, &rbo);
		if (fbo)
			glDeleteFramebuffers(1, &fbo);
	}

	FrameBuffer::FrameBuffer(int width, int height)
		: fbo{ 0 }
		, texture{ 0 }
		, rbo{ 0 }
		, width{ width }
		, height{ height }
		, clearColor{ 1.0f,1.0f,1.0f,1.0f }
	{

	}
}