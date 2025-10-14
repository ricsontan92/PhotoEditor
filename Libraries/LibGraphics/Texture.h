#pragma once

#include <string>
#include <memory>
#include <vector>

#include "LibCore/Vec4.h"

namespace LibGraphics
{
	class Texture
	{
	public:
		enum class FORMAT : int
		{
			R8 = 0,
			BGR24,	// Color texture format, 8-bits per channel.
			RGB24,	// Color texture format, 8-bits per channel.
			RGBA32, // Color with alpha texture format, 8 - bits per channel.
			YUY2	// A format that uses the YUV color space and is often used for video encoding or playback.
		};

		Texture(unsigned width, unsigned height, FORMAT format, bool mipMapped);
		Texture(const Texture& rhs) = delete;
		Texture& operator=(const Texture& rhs) = delete;
		Texture(Texture&& rhs);
		Texture& operator=(Texture&& rhs);
		~Texture();

		void Bind() const;

		uint64_t GetHandler() const { return (uint64_t)texHandler; }
		int GetHeight() const { return height; }
		int GetWidth() const { return width; }
		float GetAspect() const { return width / (float)height; }

		FORMAT GetFormat() const { return format; }
		void SetPixel(int x, int y, const LibCore::Math::Vec4& color) const;
		void SetPixels(int x, int y, int width, int height, const LibCore::Math::Vec4& color) const;
		LibCore::Math::Vec4 GetPixel(int x, int y) const;

		bool Save(const std::string& path) const;
		std::shared_ptr<Texture> Clone() const;

		// static here
		struct TextureData
		{
			FORMAT format;
			int width, height;
			std::vector<char> data;
		};
		static TextureData DecodeData(const char* data, size_t size);
		static std::shared_ptr<Texture> CreateFromFile(const std::string& filePath);
		static std::shared_ptr<Texture> CreateFromData(const std::vector<char>& data);
		static std::shared_ptr<Texture> CreateFromData(const std::vector<unsigned char>& data);
		static std::shared_ptr<Texture> CreateFromData(const std::vector<char>& data, int width, int height, FORMAT format);
		static std::shared_ptr<Texture> CreateFromData(const std::vector<unsigned char>& data, int width, int height, FORMAT format);
		static std::shared_ptr<Texture> CreateWhiteTexture(int width, int height);

	private:
		friend class FrameBuffer;
		Texture();

		int width;
		int height;
		FORMAT format;
		unsigned int texHandler;
	};
}