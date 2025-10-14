#pragma once

#include <string>
#include <vector>
#include <memory>

#include "LibCV/ImageFX.h"
#include "LibGraphics/TextureFilter.h"

struct FilterData
{
	bool Active;
	std::string Name;
	std::shared_ptr<LibGraphics::TextureFilter> Filter;
};

struct ImageProcessor
{
	ImageProcessor()
		: ImageFXFlags{
			LibCV::ImageFX::AUTO_BRIGHTNESS_CONTRAST |
			LibCV::ImageFX::AUTO_GAMMA |
			LibCV::ImageFX::AUTO_COLOR_TEMP /* |
			LibCV::ImageFX::AUTO_CLAHE |
			LibCV::ImageFX::AUTO_DETAIL_ENHANCE |
			LibCV::ImageFX::AUTO_DENOISE*/ }
		, ImageFilters{}
		, ImageSettings{}
		, Image{nullptr}
	{

	}

	unsigned ImageFXFlags;
	std::vector<FilterData> ImageFilters;
	std::vector<std::shared_ptr<LibGraphics::TextureFilter>> ImageSettings;
	std::shared_ptr<LibGraphics::Texture> Image;

	std::shared_ptr<LibGraphics::Texture> Process(const std::shared_ptr<LibGraphics::Texture>& image)
	{ 
		Image = image;

		// add settings first
		for (auto& setting : ImageSettings)
			Image = std::move(setting->Apply(Image));

		// fitlers next
		for (auto& filter : ImageFilters)
			Image = std::move(filter.Active ? filter.Filter->Apply(Image) : Image);

		return Image;
	}
};