#pragma once

#include <memory>
#include <vector>
#include <filesystem>

#include "LibCore/Vec4.h"
#include "LibCore/File.h"

namespace LibCV
{
	class ImageFX;
	struct ImageData
	{
		unsigned ImageWidth, ImageHeight, ImageChannels;
		std::vector<char> Pixels;
	};

	class Image
	{
	public:
		static std::shared_ptr<Image> Create(const LibCore::Filesystem::File& path);
		static std::shared_ptr<Image> Create(const std::filesystem::path& path);
		static std::shared_ptr<Image> Create(const char* path);
		std::shared_ptr<Image> Clone() const;
		~Image();

		void Show(const char* windowName = "default", float resize = 1.0f) const;
		void Write(const char* filename) const;

		std::shared_ptr<Image> AdjustGamma(double gamma) const;
		std::shared_ptr<Image> AutoBrightnessContrast(double clipHistPercent, double& alpha, double& beta) const;
		std::shared_ptr<Image> AdjustColorTemperature(int kelvinShift) const;
		std::shared_ptr<Image> AutoEnhance() const;

		std::shared_ptr<Image> Resize(float percentage) const;
		std::shared_ptr<Image> Resize(unsigned width, unsigned height) const;

		unsigned Width() const;
		unsigned Height() const;
		unsigned Channels() const;
		LibCore::Math::Vec4 Pixel(unsigned x, unsigned y) const;

		ImageData GetImageData() const;

	private:
		Image();

		friend class ImageFX;
		void* cvMatPtr;
	};
}