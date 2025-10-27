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
		procGLImagesPost = procGLImagesPre;

		procGLImagesPost = brightnessFilter->Apply(procGLImagesPost);
		procGLImagesPost = contrastFilter->Apply(procGLImagesPost);
		procGLImagesPost = sharpnessFilter->Apply(procGLImagesPost);
		procGLImagesPost = hslFilter->Apply(procGLImagesPost);
		procGLImagesPost = temperatureFilter->Apply(procGLImagesPost);
		procGLImagesPost = gammaFilter->Apply(procGLImagesPost);

		// apply filters
		for (auto& filter : imageFilters)
			procGLImagesPost = std::move(filter->Active ? filter->Filter->Apply(procGLImagesPost) : procGLImagesPost);
	}
}

void ImageProcessor::Update()
{
	if (loadImageFuture.IsReady())
	{
		loadImageFuture.Get();

		// processed image data into GPU
		const auto imageData = procCVImage->GetImageData();
		procGLImagesPre = LibGraphics::Texture::CreateFromData(
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
}

bool ImageProcessor::LoadImage(const LibCore::Filesystem::Path& path)
{
	if (path.Exists())
	{
		origGLImage = procGLImagesPre = procGLImagesPost = nullptr;
		loadImageFuture = LibCore::Async::Run([this, path]() {
			origCVImage = LibCV::Image::Create(path.String().c_str());
			procCVImage = LibCV::ImageFX::AutoEnhance(IMAGE_REDUCER(origCVImage), imageFXFlags);
		});

		return true;
	}
	return false;
}

bool ImageProcessor::IsLoadImageCompleted()
{
	return !loadImageFuture.Valid() || GetProcessedGLImage() != nullptr;
}


LibCore::Async::Future<LibCV::ImageData> ImageProcessor::GenCVEnhancedImage(
	LibCore::Async::ThreadPool& threadPool,
	const LibCore::Filesystem::File& filePath) const
{
	return threadPool.Enqueue([filePath](unsigned fxFlags) {
		auto results = LibCV::Image::Create(filePath.FilePath().String());
		results = LibCV::ImageFX::AutoEnhance(results, fxFlags);
		return results->GetImageData();
	}, imageFXFlags);
}

std::shared_ptr<LibGraphics::Texture> ImageProcessor::GenGLEnhancedImage(const LibCV::ImageData& imageData) const
{
	auto results = LibGraphics::Texture::CreateFromData(
		imageData.Pixels,
		imageData.ImageWidth,
		imageData.ImageHeight,
		LibGraphics::Texture::FORMAT::BGR24);

	results = brightnessFilter->Apply(results);
	results = contrastFilter->Apply(results);
	results = sharpnessFilter->Apply(results);
	results = hslFilter->Apply(results);
	results = temperatureFilter->Apply(results);
	results = gammaFilter->Apply(results);

	// apply filters
	for (auto& filter : imageFilters)
		results = std::move(filter->Active ? filter->Filter->Apply(results) : results);

	return results;
}

const std::shared_ptr<LibGraphics::Texture>& ImageProcessor::GetProcessedGLImage() const
{
	return procGLImagesPost;
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
		origGLImage = procGLImagesPre = procGLImagesPost = nullptr;
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

std::shared_ptr< ImageProcessor> ImageProcessor::Clone() const
{
	auto results = std::make_shared<ImageProcessor>();

	for (auto& filter : imageFilters)
	{
		results->imageFilters.push_back({});
		results->imageFilters.back()->Name = filter->Name;
		results->imageFilters.back()->Active = filter->Active;
		results->imageFilters.back()->Filter = filter->Filter->Clone();
	}

	results->sharpnessFilter->SetFloat("uSharpness", GetImageSetting(IMAGE_SETTINGS::SHARPNESS));
	results->contrastFilter->SetFloat("uContrast", GetImageSetting(IMAGE_SETTINGS::CONSTRAST));
	results->brightnessFilter->SetFloat("uBrightness", GetImageSetting(IMAGE_SETTINGS::BRIGHTNESS));
	results->hslFilter->SetFloat("uHue", GetImageSetting(IMAGE_SETTINGS::HUE));
	results->hslFilter->SetFloat("uSaturation", GetImageSetting(IMAGE_SETTINGS::SATURATION));
	results->hslFilter->SetFloat("uLightness", GetImageSetting(IMAGE_SETTINGS::LIGHTNESS));
	results->temperatureFilter->SetFloat("uTemperature", GetImageSetting(IMAGE_SETTINGS::TEMPERATURE));
	results->gammaFilter->SetFloat("uGamma", GetImageSetting(IMAGE_SETTINGS::GAMMA));

	results->imageFXFlags = imageFXFlags;

	results->procCVImage = this->procCVImage->Clone();
	results->origCVImage = this->origCVImage->Clone();

	// processed image data into GPU
	const auto imageData = results->procCVImage->GetImageData();
	results->procGLImagesPre = LibGraphics::Texture::CreateFromData(
		imageData.Pixels,
		imageData.ImageWidth,
		imageData.ImageHeight,
		LibGraphics::Texture::FORMAT::BGR24);

	// convert to GPU for purely loaded image
	const auto origImageData = results->origCVImage->Resize(static_cast<float>(results->procCVImage->Width()) / results->origCVImage->Width())->GetImageData();
	results->origGLImage = LibGraphics::Texture::CreateFromData(
		origImageData.Pixels,
		origImageData.ImageWidth,
		origImageData.ImageHeight,
		LibGraphics::Texture::FORMAT::BGR24);

	results->ProcessGLChanges();

	return results;
}