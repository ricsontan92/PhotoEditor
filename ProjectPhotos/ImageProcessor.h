#pragma once

#include <string>
#include <vector>
#include <memory>

#include "LibCore/Path.h"
#include "LibCore/Future.h"
#include "LibCore/ThreadPool.h"

#include "LibCV/ImageFX.h"

#include "LibGraphics/TextureFilter.h"
#include "LibGraphics/DefaultShaders.h"

struct FilterData
{
	bool Active;
	std::string Name;
	std::shared_ptr<LibGraphics::TextureFilter> Filter;
};

class ImageProcessor
{
public:
	enum class IMAGE_SETTINGS
	{
		BRIGHTNESS,
		CONSTRAST,
		SHARPNESS,
		HUE,
		SATURATION,
		LIGHTNESS,
		TEMPERATURE,
		GAMMA
	};

	ImageProcessor()
		: imageFXFlags{
			LibCV::ImageFX::AUTO_BRIGHTNESS_CONTRAST |
			LibCV::ImageFX::AUTO_GAMMA |
			LibCV::ImageFX::AUTO_COLOR_TEMP /* |
			LibCV::ImageFX::AUTO_HSL |
			LibCV::ImageFX::AUTO_CLAHE |
			LibCV::ImageFX::AUTO_DETAIL_ENHANCE |
			LibCV::ImageFX::AUTO_DENOISE*/ }
		, imageFilters{}
		, sharpnessFilter{ nullptr }
		, contrastFilter{ nullptr }
		, brightnessFilter{ nullptr }
		, hslFilter{ nullptr }
		, temperatureFilter{ nullptr }
		, gammaFilter{ nullptr }
		, procCVImage{ nullptr }
		, origGLImage{ nullptr }
		, procGLImagesPre{ nullptr }
		, procGLImagesPost{ nullptr }
	{
		brightnessFilter	= LibGraphics::TextureFilter::CreateFromShader(LibGraphics::BRIGHTNESS_SHADER);
		contrastFilter		= LibGraphics::TextureFilter::CreateFromShader(LibGraphics::CONTRAST_SHADER);
		sharpnessFilter		= LibGraphics::TextureFilter::CreateFromShader(LibGraphics::SHARPEN_SHADER);
		hslFilter			= LibGraphics::TextureFilter::CreateFromShader(LibGraphics::HSL_ADJUSTMENT_SHADER);
		temperatureFilter	= LibGraphics::TextureFilter::CreateFromShader(LibGraphics::TEMPERATURE_SHADER);
		gammaFilter			= LibGraphics::TextureFilter::CreateFromShader(LibGraphics::GAMMA_SHADER);
	}

	void Update();
	bool LoadImage(const LibCore::Filesystem::Path& path);
	bool IsLoadImageCompleted();

	LibCore::Async::Future<LibCV::ImageData> GenCVEnhancedImage(
		LibCore::Async::ThreadPool& threadPool,
		const LibCore::Filesystem::File& filePath) const;

	// MUST BE in main thread
	std::shared_ptr<LibGraphics::Texture> GenGLEnhancedImage(const LibCV::ImageData& imgData) const;

	const std::shared_ptr<LibGraphics::Texture>& GetProcessedGLImage() const;
	const std::shared_ptr<LibGraphics::Texture>& GetOriginalGLImage() const;

	std::shared_ptr<FilterData> AddImageFilters(
		const std::string& filterName,
		const std::shared_ptr<LibGraphics::TextureFilter>& textureFilters);
	const std::vector<std::shared_ptr<FilterData>>& GetImageFilters() const;
	void SetImageFilters(const std::vector<std::shared_ptr<FilterData>>& filters);
	void RemoveImageFilters(const std::shared_ptr<FilterData>& filter);

	void SetImageSettingDefaults();
	float GetImageSetting(IMAGE_SETTINGS setting) const;
	LibCore::Math::Vec2 GetImageSettingRange(IMAGE_SETTINGS setting) const;
	void SetImageSetting(IMAGE_SETTINGS setting, float value);

	unsigned GetFXFlags() const;
	void SetFXFlags(unsigned flags);

	unsigned GetImageHeight() const;
	unsigned GetImageWidth() const;
	std::shared_ptr< ImageProcessor> Clone() const;

private:
	unsigned imageFXFlags;

private:
	std::vector<std::shared_ptr<FilterData>> imageFilters;
	
private:
	friend class ImageProcessingExecutor;

	void ProcessGLChanges();
	std::shared_ptr<LibGraphics::TextureFilter> sharpnessFilter;
	std::shared_ptr<LibGraphics::TextureFilter> contrastFilter;
	std::shared_ptr<LibGraphics::TextureFilter> brightnessFilter;
	std::shared_ptr<LibGraphics::TextureFilter> hslFilter;
	std::shared_ptr<LibGraphics::TextureFilter> temperatureFilter;
	std::shared_ptr<LibGraphics::TextureFilter> gammaFilter;

private:
	LibCore::Async::Future<void> loadImageFuture;

	std::shared_ptr<LibCV::Image> origCVImage, procCVImage;
	std::shared_ptr<LibGraphics::Texture> origGLImage, procGLImagesPre, procGLImagesPost;
};