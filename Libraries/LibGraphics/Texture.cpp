#include "Texture.h"
#include "GL/glew.h"
#include "soil/SOIL.h"
#include "soil/image_helper.h"

#include <array>
#include <filesystem>

namespace LibGraphics
{
	Texture::Texture() : Texture(0, 0, FORMAT::RGB24, true)
	{

	}

	Texture::Texture(unsigned wd, unsigned ht, FORMAT f, bool mipMapped)
		: texHandler(0)
		, width(wd)
		, height(ht)
		, format(f)
	{
		if (width == 0 || height == 0)
			return;

		int channels = 3;
		unsigned type = 0;
		switch (format)
		{
		case FORMAT::RGB24:
			type = GL_RGB;
			break;
		case FORMAT::RGBA32:
			type = GL_RGBA;
			channels = 4;
			break;
		default:
			throw "Not yet implemented";
			break;
		}

		glActiveTexture(GL_TEXTURE0);
		glGenTextures(1, &texHandler);
		glBindTexture(GL_TEXTURE_2D, texHandler);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

		std::vector<unsigned char> img(static_cast<size_t>(height * width * channels), 255);

		glTexImage2D(GL_TEXTURE_2D, 0, type, width, height, 0, type, GL_UNSIGNED_BYTE, img.data());
	}

	Texture::Texture(Texture&& rhs)
		: width(rhs.width)
		, height(rhs.height)
		, format(rhs.format)
		, texHandler(rhs.texHandler)
	{
		rhs.texHandler = 0;
	}

	Texture& Texture::operator=(Texture&& rhs)
	{
		std::swap(rhs.width, width);
		std::swap(rhs.height, height);
		std::swap(rhs.format, format);
		std::swap(rhs.texHandler, texHandler);
		return *this;
	}

	Texture::~Texture()
	{
		if (texHandler)
		{
			glDeleteTextures(1, &texHandler);
			texHandler = 0;
		}
	}

	void Texture::Bind() const
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texHandler);
	}

	void Texture::SetPixel(int x, int y, const LibCore::Math::Vec4& color) const
	{
		unsigned type = 0;
		switch (format)
		{
		case FORMAT::RGB24:
			type = GL_RGB;
			break;
		case FORMAT::RGBA32:
			type = GL_RGBA;
			break;
		default:
			throw "Not yet implemented";
			break;
		}

		unsigned char colorBuf[4]{
			static_cast<unsigned char>(color.x * 255),
			static_cast<unsigned char>(color.y * 255),
			static_cast<unsigned char>(color.z * 255),
			static_cast<unsigned char>(color.w * 255)
		};

		glTextureSubImage2D(texHandler,
			0,					// level 
			x, y,				// offsets
			1, 1,				// size
			type,				// type
			GL_UNSIGNED_BYTE,
			colorBuf);
	}

	void Texture::SetPixels(int x, int y, int width, int height, const LibCore::Math::Vec4& color) const
	{
		if (format == FORMAT::RGB24)
		{
			std::array<unsigned char, 3> data
			{
				static_cast<unsigned char>(color.x * 255),
				static_cast<unsigned char>(color.y * 255),
				static_cast<unsigned char>(color.z * 255)
			};

			std::vector<std::array<unsigned char, 3>> buffer(size_t(width * height), data);

			glTextureSubImage2D(texHandler,
				0,					// level 
				x, y,				// offsets
				width, height,		// size
				GL_RGB,				// type
				GL_UNSIGNED_BYTE,
				buffer.data());
		}
		else if (format == FORMAT::RGBA32)
		{
			std::array<unsigned char, 4> data
			{
				static_cast<unsigned char>(color.x * 255),
				static_cast<unsigned char>(color.y * 255),
				static_cast<unsigned char>(color.z * 255),
				static_cast<unsigned char>(color.w * 255)
			};

			std::vector<std::array<unsigned char, 4>> buffer(size_t(width * height), data);

			glTextureSubImage2D(texHandler,
				0,					// level 
				x, y,				// offsets
				width, height,		// size
				GL_RGBA,			// type
				GL_UNSIGNED_BYTE,
				buffer.data());
		}
		else
		{
			throw "Not yet implemented";
		}
	}

	LibCore::Math::Vec4 Texture::GetPixel(int x, int y) const
	{
		GLuint fboID;
		glGenFramebuffers(1, &fboID);  // Generate the framebuffer object
		glBindFramebuffer(GL_FRAMEBUFFER, fboID);  // Bind the FBO

		// Attach the texture to the FBO (as a color attachment)
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texHandler, 0);

		GLubyte pixel[4];  // Store the RGBA value
		glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
		auto results = LibCore::Math::Vec4{ (float)pixel[0], (float)pixel[1], (float)pixel[2], (float)pixel[3] } / 255.0f;

		glDeleteFramebuffers(1, &fboID);

		return results;
	}

	bool Texture::Save(const std::string& path) const
	{
		std::string ext = std::filesystem::path{ path }.extension().string();
		int channels = format == FORMAT::RGBA32 ? 4 : 3;
		std::vector<unsigned char> pixels((size_t)GetWidth() * GetHeight() * channels, 0);
		Bind();
		glGetTexImage(GL_TEXTURE_2D, 0, channels == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

		return SOIL_save_image(
			path.c_str(),
			ext == ".bmp" ? SOIL_SAVE_TYPE_BMP : (ext == ".tga" ? SOIL_SAVE_TYPE_TGA : SOIL_SAVE_TYPE_DDS),
			GetWidth(),
			GetHeight(),
			channels,
			pixels.data());
	}

	std::shared_ptr<Texture> Texture::Clone() const
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
			texHandler, GL_TEXTURE_2D, 0, 0, 0, 0,		// Source texture parameters
			result->texHandler, GL_TEXTURE_2D, 0, 0, 0, 0,	// Destination texture parameters
			result->width, result->height, 1				// Dimensions of the copied region
		);
		return result;
	}

	std::shared_ptr<Texture> Texture::CreateWhiteTexture(int width, int height)
	{
		std::shared_ptr<Texture> result{ new Texture{(unsigned)width, (unsigned)height, FORMAT::RGB24, false} };
		return result;
	}

	std::shared_ptr<Texture> Texture::CreateFromData(const std::vector<char>& data)
	{
		int width, height, channel;
		unsigned char* pData = SOIL_load_image_from_memory(
			(unsigned char*)data.data(),
			(int)data.size(),
			&width,
			&height,
			&channel,
			SOIL_LOAD_AUTO);

		std::shared_ptr<Texture> result{ new Texture{} };

		{
			int new_width = 1;
			int new_height = 1;
			while (new_width < width) new_width *= 2;
			while (new_height < height) new_height *= 2;

			if (new_width != width || new_height != height)
			{
				unsigned char* resampled = (unsigned char*)malloc(channel * new_width * new_height);
				up_scale_image(
					pData, width, height, channel,
					resampled, new_width, new_height);
				SOIL_free_image_data(pData);
				pData = resampled;
				width = new_width;
				height = new_height;
			}
		}

		// texturing here
		glActiveTexture(GL_TEXTURE0);
		glGenTextures(1, &result->texHandler);
		glBindTexture(GL_TEXTURE_2D, result->texHandler);

		// set parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		result->format = channel == 3 ? FORMAT::RGB24 : FORMAT::RGBA32;
		result->width = width;
		result->height = height;

		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			channel == 3 ? GL_RGB : GL_RGBA,
			result->width,
			result->height,
			0,
			channel == 3 ? GL_RGB : GL_RGBA,
			GL_UNSIGNED_BYTE,
			pData);

		glGenerateMipmap(GL_TEXTURE_2D);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);

		SOIL_free_image_data(pData);

		return result;
	}

	std::shared_ptr<Texture> Texture::CreateFromData(const std::vector<unsigned char>& data)
	{
		int width, height, channel;
		unsigned char* pData = SOIL_load_image_from_memory(
			(unsigned char*)data.data(),
			(int)data.size(),
			&width,
			&height,
			&channel,
			SOIL_LOAD_AUTO);

		std::shared_ptr<Texture> result{ new Texture{} };

		{
			int new_width = 1;
			int new_height = 1;
			while (new_width < width) new_width *= 2;
			while (new_height < height) new_height *= 2;

			if (new_width != width || new_height != height)
			{
				unsigned char* resampled = (unsigned char*)malloc(channel * new_width * new_height);
				up_scale_image(
					pData, width, height, channel,
					resampled, new_width, new_height);
				SOIL_free_image_data(pData);
				pData = resampled;
				width = new_width;
				height = new_height;
			}
		}

		// texturing here
		glActiveTexture(GL_TEXTURE0);
		glGenTextures(1, &result->texHandler);
		glBindTexture(GL_TEXTURE_2D, result->texHandler);

		// set parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		result->format = channel == 3 ? FORMAT::RGB24 : FORMAT::RGBA32;
		result->width = width;
		result->height = height;

		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			channel == 3 ? GL_RGB : GL_RGBA,
			result->width,
			result->height,
			0,
			channel == 3 ? GL_RGB : GL_RGBA,
			GL_UNSIGNED_BYTE,
			pData);

		glGenerateMipmap(GL_TEXTURE_2D);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);

		SOIL_free_image_data(pData);

		return result;
	}

	std::shared_ptr<Texture> Texture::CreateFromData(const std::vector<char>& data, int width, int height, FORMAT format)
	{
		std::shared_ptr<Texture> result{ new Texture{} };

		// texturing here
		glActiveTexture(GL_TEXTURE0);
		glGenTextures(1, &result->texHandler);
		glBindTexture(GL_TEXTURE_2D, result->texHandler);

		// set parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		unsigned glFormat = 0;
		unsigned glInternalFormat = 0;
		switch (format)
		{
		case FORMAT::R8:
			glFormat = GL_RED;
			glInternalFormat = GL_RED;
			break;
		case FORMAT::BGR24:
			glFormat = GL_BGR;
			glInternalFormat = GL_RGB;
			break;
		case FORMAT::RGB24:
			glFormat = GL_RGB;
			glInternalFormat = GL_RGB;
			break;
		case FORMAT::RGBA32:
			glFormat = GL_RGBA;
			glInternalFormat = GL_RGBA;
			break;
		default:
			throw std::exception("Texture channel not implemented");
		}

		result->format = format;
		result->width = width;
		result->height = height;

		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			glInternalFormat,
			result->width,
			result->height,
			0,
			glFormat,
			GL_UNSIGNED_BYTE,
			data.data());
		glGenerateMipmap(GL_TEXTURE_2D);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);

		return result;
	}

	std::shared_ptr<Texture> Texture::CreateFromData(const std::vector<unsigned char>& data, int width, int height, FORMAT format)
	{
		std::shared_ptr<Texture> result{ new Texture{} };

		// texturing here
		glActiveTexture(GL_TEXTURE0);
		glGenTextures(1, &result->texHandler);
		glBindTexture(GL_TEXTURE_2D, result->texHandler);

		// set parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		unsigned glFormat = 0;
		unsigned glInternalFormat = 0;
		switch (format)
		{
		case FORMAT::R8:
			glFormat = GL_RED;
			glInternalFormat = GL_RED;
			break;
		case FORMAT::BGR24:
			glFormat = GL_BGR;
			glInternalFormat = GL_RGB;
			break;
		case FORMAT::RGB24:
			glFormat = GL_RGB;
			glInternalFormat = GL_RGB;
			break;
		case FORMAT::RGBA32:
			glFormat = GL_RGBA;
			glInternalFormat = GL_RGBA;
			break;
		default:
			throw std::exception("Texture channel not implemented");
		}

		result->format = format;
		result->width = width;
		result->height = height;

		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			glInternalFormat,
			result->width,
			result->height,
			0,
			glFormat,
			GL_UNSIGNED_BYTE,
			data.data());
		glGenerateMipmap(GL_TEXTURE_2D);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);

		return result;
	}

	Texture::TextureData Texture::DecodeData(const char* data, size_t size)
	{
		int channel;
		Texture::TextureData texture;
		unsigned char* pData = SOIL_load_image_from_memory(
			(unsigned char*)data,
			(int)size,
			&texture.width,
			&texture.height,
			&channel,
			SOIL_LOAD_AUTO);

		if (!pData)
		{
			return texture;
		}

		{
			int new_width = 1;
			int new_height = 1;
			while (new_width < texture.width) new_width *= 2;
			while (new_height < texture.height) new_height *= 2;

			if (new_width != texture.width || new_height != texture.height)
			{
				std::vector<char> resampled;
				resampled.resize(channel * new_width * new_height);
				up_scale_image(
					pData, texture.width, texture.height, channel,
					(unsigned char*)resampled.data(), new_width, new_height);
				texture.data = std::move(resampled);
				texture.width = new_width;
				texture.height = new_height;
			}
			else
				texture.data = std::vector<char>{ pData, pData + texture.width * texture.height * channel };
		}
		SOIL_free_image_data(pData);

		texture.format = channel == 3 ? FORMAT::RGB24 : FORMAT::RGBA32;

		return texture;
	}

	std::shared_ptr<Texture> Texture::CreateFromFile(const std::string& filePath)
	{
		std::shared_ptr<Texture> result{ new Texture{} };

		// texturing here
		int channel;
		glActiveTexture(GL_TEXTURE0);
		glGenTextures(1, &result->texHandler);
		glBindTexture(GL_TEXTURE_2D, result->texHandler);
		unsigned char* pData = SOIL_load_image(filePath.c_str(), &result->width, &result->height, &channel, SOIL_LOAD_AUTO);

		if (pData == nullptr)
		{
			std::string error_msg = "An error occurred while loading " + filePath + ". Reason: " + SOIL_last_result() + ".";
			throw std::exception(error_msg.c_str());
		}

		// set parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		unsigned format = 0;

		// channel = 1: luminious
		// channel = 2: luminious with alpha
		// channel = 3: rgb
		// channel = 4: rgba
		switch (channel)
		{
		case 3:
			format = GL_RGB;
			result->format = FORMAT::RGB24;
			break;
		case 4:
			format = GL_RGBA;
			result->format = FORMAT::RGBA32;
			break;
		default:
			throw std::exception("Texture channel not implemented");
		}

		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			format,
			result->width,
			result->height,
			0,
			format,
			GL_UNSIGNED_BYTE,
			pData);
		glGenerateMipmap(GL_TEXTURE_2D);

		SOIL_free_image_data(pData);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);

		return result;
	}
}