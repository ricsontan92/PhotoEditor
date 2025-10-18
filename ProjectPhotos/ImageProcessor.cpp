#include "ImageProcessor.h"

#define IMAGE_REDUCER(image) image->Resize(std::max(image->Width() > 1920 || image->Height() > 1080 \
						? (image->Width() > image->Height() ? 1920.0f / image->Width() : 1080.0f / image->Height()) \
						: 1.0f, 0.5f))

//#define IMAGE_REDUCER(image) image

void ImageProcessor::ProcessGLChanges()
{
	if (procCVImage)
	{
		// apply settings
		procGLImages[1] = brightnessFilter->Apply(procGLImages[0]);
		procGLImages[1] = contrastFilter->Apply(procGLImages[1]);
		procGLImages[1] = sharpnessFilter->Apply(procGLImages[1]);
		procGLImages[1] = hslFilter->Apply(procGLImages[1]);
		procGLImages[1] = temperatureFilter->Apply(procGLImages[1]);
		procGLImages[1] = gammaFilter->Apply(procGLImages[1]);

		// apply filters
		for (auto& filter : imageFilters)
			procGLImages[1] = std::move(filter->Active ? filter->Filter->Apply(procGLImages[1]) : procGLImages[1]);
	}
}

bool ImageProcessor::LoadImage(const LibCore::Filesystem::Path& path)
{
	if (path.Exists())
	{
		loadImageFuture = LibCore::Async::Run([this, path]() {
			origCVImage = LibCV::Image::Create(path.String().c_str());
			procCVImage = LibCV::ImageFX::AutoEnhance(IMAGE_REDUCER(origCVImage), imageFXFlags);
		});

		return true;
	}
	return false;
}

bool ImageProcessor::TestLoadImageCompleted()
{
	if (loadImageFuture.IsReady())
	{
		loadImageFuture.Get();

		const auto imageData = procCVImage->GetImageData();

		procGLImages[0] = LibGraphics::Texture::CreateFromData(
			imageData.Pixels,
			imageData.ImageWidth,
			imageData.ImageHeight,
			LibGraphics::Texture::FORMAT::BGR24);

		// convert to GPU for purely loaded image
		const auto origImageData = origCVImage->Resize(static_cast<float>(procCVImage->Width()) / origCVImage->Width())->GetImageData();
		origGLImage = LibGraphics::Texture::CreateFromData(
			origImageData.Pixels,
			origImageData.ImageWidth,
			origImageData.ImageHeight,
			LibGraphics::Texture::FORMAT::BGR24);

		// convert to GPU for enhanced by CV image
		ProcessGLChanges();
	}

	return !loadImageFuture.Valid() || GetProcessedGLImage() != nullptr;
}

const std::shared_ptr<LibGraphics::Texture>& ImageProcessor::GetProcessedGLImage() const
{
	return procGLImages[1];
}

const std::shared_ptr<LibGraphics::Texture>& ImageProcessor::GetOriginalGLImage() const
{
	return origGLImage;
}

std::shared_ptr<FilterData> ImageProcessor::AddImageFilters(
	const std::string& filterName,
	const std::shared_ptr<LibGraphics::TextureFilter>& textureFilters)
{
	imageFilters.push_back(std::make_shared<FilterData>());
	imageFilters.back()->Active = true;
	imageFilters.back()->Name = filterName;
	imageFilters.back()->Filter = textureFilters->Clone();

	ProcessGLChanges();

	return imageFilters.back();
}

void ImageProcessor::SetImageFilters(const std::vector<std::shared_ptr<FilterData>>& filters)
{
	imageFilters = filters;
	ProcessGLChanges();
}

const std::vector<std::shared_ptr<FilterData>>& ImageProcessor::GetImageFilters() const
{
	return imageFilters;
}

void ImageProcessor::RemoveImageFilters(const std::shared_ptr<FilterData>& filter)
{
	auto it = std::find(imageFilters.begin(), imageFilters.end(), filter);
	if (it != imageFilters.end())
	{
		imageFilters.erase(it);
		ProcessGLChanges();
	}
}

void ImageProcessor::SetImageSettingDefaults()
{
	brightnessFilter->SetFloat("uBrightness",	0.0f);
	contrastFilter->SetFloat("uContrast",		1.0f);
	sharpnessFilter->SetFloat("uSharpness",		0.0f);
	hslFilter->SetFloat("uHue",					0.0f);
	hslFilter->SetFloat("uSaturation",			1.0f);
	hslFilter->SetFloat("uLightness",			0.5f);
	temperatureFilter->SetFloat("uTemperature", 0.0f);
	gammaFilter->SetFloat("uGamma",				1.0f);

	ProcessGLChanges();
}

float ImageProcessor::GetImageSetting(IMAGE_SETTINGS setting) const
{
	float data = -1.0f;
	switch (setting)
	{
	case IMAGE_SETTINGS::BRIGHTNESS:
		return brightnessFilter->GetFloat("uBrightness", data) ? data : -1.0f;
	case IMAGE_SETTINGS::CONSTRAST:
		return contrastFilter->GetFloat("uContrast", data) ? data : -1.0f;
	case IMAGE_SETTINGS::SHARPNESS:
		return sharpnessFilter->GetFloat("uSharpness", data) ? data : -1.0f;
	case IMAGE_SETTINGS::HUE:
		return hslFilter->GetFloat("uHue", data) ? data : -1.0f;
	case IMAGE_SETTINGS::SATURATION:
		return hslFilter->GetFloat("uSaturation", data) ? data : -1.0f;
	case IMAGE_SETTINGS::LIGHTNESS:
		return hslFilter->GetFloat("uLightness", data) ? data : -1.0f;
	case IMAGE_SETTINGS::TEMPERATURE:
		return temperatureFilter->GetFloat("uTemperature", data) ? data : -1.0f;
	case IMAGE_SETTINGS::GAMMA:
		return gammaFilter->GetFloat("uGamma", data) ? data : -1.0f;
	default:
		return data;
	}
}

LibCore::Math::Vec2 ImageProcessor::GetImageSettingRange(IMAGE_SETTINGS setting) const
{
	LibCore::Math::Vec2 data = { -1.0f, -1.0f };
	switch (setting)
	{
	case IMAGE_SETTINGS::BRIGHTNESS:
		return LibCore::Math::Vec2{ -1.0f, +1.0f };
	case IMAGE_SETTINGS::CONSTRAST:
		return LibCore::Math::Vec2{ +0.0f, +2.0f };
	case IMAGE_SETTINGS::SHARPNESS:
		return LibCore::Math::Vec2{ -2.0f, +2.0f };
	case IMAGE_SETTINGS::HUE:
		return LibCore::Math::Vec2{ -360.0f, +360.0f };
	case IMAGE_SETTINGS::SATURATION:
		return LibCore::Math::Vec2{ +0.0f, +2.0f };
	case IMAGE_SETTINGS::LIGHTNESS:
		return LibCore::Math::Vec2{ +0.0f, +1.0f };
	case IMAGE_SETTINGS::TEMPERATURE:
		return LibCore::Math::Vec2{ -1.0f, +1.0f };
	case IMAGE_SETTINGS::GAMMA:
		return LibCore::Math::Vec2{ +0.0f, +2.0f };
	default:
		return data;
	}
}

void ImageProcessor::SetImageSetting(IMAGE_SETTINGS setting, float value)
{
	switch (setting)
	{
	case IMAGE_SETTINGS::BRIGHTNESS:
		brightnessFilter->SetFloat("uBrightness", value);
		break;
	case IMAGE_SETTINGS::CONSTRAST:
		contrastFilter->SetFloat("uContrast", value);
		break;
	case IMAGE_SETTINGS::SHARPNESS:
		sharpnessFilter->SetFloat("uSharpness", value);
		break;
	case IMAGE_SETTINGS::HUE:
		hslFilter->SetFloat("uHue", value);
		break;
	case IMAGE_SETTINGS::SATURATION:
		hslFilter->SetFloat("uSaturation", value);
		break;
	case IMAGE_SETTINGS::LIGHTNESS:
		hslFilter->SetFloat("uLightness", value);
		break;
	case IMAGE_SETTINGS::TEMPERATURE:
		temperatureFilter->SetFloat("uTemperature", value);
		break;
	case IMAGE_SETTINGS::GAMMA:
		gammaFilter->SetFloat("uGamma", value);
		break;
	default: break;
	}

	ProcessGLChanges();
}

unsigned ImageProcessor::GetFXFlags() const
{
	return imageFXFlags;
}

void ImageProcessor::SetFXFlags(unsigned flags)
{
	bool isDiff = imageFXFlags != flags;

	imageFXFlags = flags;

	if (isDiff)
	{
		procGLImages[0] = procGLImages[1] = nullptr;
		loadImageFuture = LibCore::Async::Run([this, flags]() {
			procCVImage = LibCV::ImageFX::AutoEnhance(IMAGE_REDUCER(origCVImage), flags);
		});
	}
}

unsigned ImageProcessor::GetImageHeight() const
{
	return origCVImage ? origCVImage->Height() : 0;
}

unsigned ImageProcessor::GetImageWidth() const
{
	return origCVImage ? origCVImage->Width() : 0;
}