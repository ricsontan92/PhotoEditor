#pragma once
#include "Image.h"

namespace LibCV
{
	class ImageFX 
	{
	public:
		const static unsigned AUTO_BRIGHTNESS_CONTRAST	= 1 << 0;
		const static unsigned AUTO_GAMMA				= 1 << 1;
		const static unsigned AUTO_COLOR_TEMP			= 1 << 2;
		const static unsigned AUTO_CLAHE				= 1 << 3;
		const static unsigned AUTO_DETAIL_ENHANCE		= 1 << 4;
		const static unsigned AUTO_DENOISE				= 1 << 5;

		static std::shared_ptr<Image> AdjustBrightnessContrast(const std::shared_ptr<Image>& image, double clipHistPercent);
		static std::shared_ptr<Image> AdjustGamma(const std::shared_ptr<Image>& image, double gamma);
		static std::shared_ptr<Image> AdjustColorTemperature(const std::shared_ptr<Image>& image, int kelvinShift);
		static std::shared_ptr<Image> ApplyCLAHE(const std::shared_ptr<Image>& image);
		static std::shared_ptr<Image> ApplyEnhanceDetails(const std::shared_ptr<Image>& image);
		static std::shared_ptr<Image> ApplyPencilSketch(const std::shared_ptr<Image>& image, bool gray);
		static std::shared_ptr<Image> ApplyDenoise(const std::shared_ptr<Image>& image);
		static std::shared_ptr<Image> AutoEnhance(const std::shared_ptr<Image>& image, unsigned flags);

	private:
	};
}