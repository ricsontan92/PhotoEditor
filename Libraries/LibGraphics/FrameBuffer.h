#pragma once
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "Texture.h"
#include "LibCore/Vec4.h"

namespace LibGraphics
{
	class FrameBuffer
	{
	public:
		static std::shared_ptr< FrameBuffer > CreateFrameBuffer(int width, int height);
		static std::shared_ptr< FrameBuffer > CreateFrameBuffer(int width, int height, int targets);
		~FrameBuffer();

		std::vector< std::vector<float>> ReadMultiTargetPixelsAsFloats(int targetNum) const;
		std::vector< std::vector<unsigned char>> ReadMultiTargetPixels(int targetNum) const;
		std::vector<unsigned char> ReadPixels(int target = 0) const;	// format is in BGR
		std::vector<float> ReadPixelsAsFloats(int target = 0) const;	// format is in BGR
		unsigned GetGLTextureID(int target = 0) const;
		std::shared_ptr< Texture > GetGLTexture(int target = 0) const;
		void BindTexture(int target = 0) const;
		float GetAspect() const;
		bool Save(const std::string& path) const;

		int GetWidth() const;
		int GetHeight() const;
		void Bind() const;
		void Unbind() const;
		void Resize(int width, int height);
		void SetClearColor(const LibCore::Math::Vec4& clearColor);
		void RenderToBuffer(const std::function<void()>& renderCall) const;

	private:
		FrameBuffer(int width, int height);
		int width;
		int height;
		unsigned fbo, rbo;
		LibCore::Math::Vec4 clearColor;
		std::vector<unsigned int> texture;
	};
}