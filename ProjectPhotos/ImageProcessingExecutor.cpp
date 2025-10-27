#include "ImageProcessingExecutor.h"

std::shared_ptr<ImageProcessingExecutor> ImageProcessingExecutor::Run(
	const std::shared_ptr< ImageProcessor>& processor,
	const std::vector<LibCore::Filesystem::File>& imageFiles,
	const LibCore::Filesystem::Directory& saveDirectory
)
{
	auto results = std::shared_ptr<ImageProcessingExecutor>(new ImageProcessingExecutor{});
	
	if (!saveDirectory.Exists())
		saveDirectory.Create();

	results->saveDirectory = saveDirectory;

	results->imageFilters.push_back(processor->brightnessFilter->Clone());
	results->imageFilters.push_back(processor->contrastFilter->Clone());
	results->imageFilters.push_back(processor->sharpnessFilter->Clone());
	results->imageFilters.push_back(processor->hslFilter->Clone());
	results->imageFilters.push_back(processor->temperatureFilter->Clone());
	results->imageFilters.push_back(processor->gammaFilter->Clone());

	for (auto& filter : processor->imageFilters)
	{
		if(filter->Active)
			results->imageFilters.push_back(filter->Filter->Clone());
	}

	for (auto& file : imageFiles)
	{
		if (!file.Exists() || results->enhanceImageFutures.find(file.FileName()) != results->enhanceImageFutures.end())
			continue;

		auto fut = results->imageEnhanceThreadPool.Enqueue([results, file](unsigned imageFxFlags) {
			auto image = LibCV::Image::Create(file);
			if (image)
			{
				image = LibCV::ImageFX::AutoEnhance(image, imageFxFlags);
				return image->GetImageData();
			}
			return LibCV::ImageData{ 0, 0, 0, {}  };
		}, processor->imageFXFlags);

		++results->totalImages;
		results->enhanceImageFutures[file.FileName()] = std::move(fut);
	}

	return results;
}

ImageProcessingExecutor::ImageProcessingExecutor()
	: totalImages{ 0 }
	, completedImages{ 0 }
	, imageFilters{ }
	, imageSaveThreadPool{ 2 }
	, imageEnhanceThreadPool{ std::max(std::thread::hardware_concurrency() >> 2, 3U)}
{

}

ImageProcessingExecutor::~ImageProcessingExecutor()
{

}

void ImageProcessingExecutor::Update()
{
	for (auto it = enhanceImageFutures.begin(); it != enhanceImageFutures.end(); ++it)
	{
		if (it->second.IsReady())
		{
			const auto imageData = it->second.Get();

			auto glImage = LibGraphics::Texture::CreateFromData(
				imageData.Pixels,
				imageData.ImageWidth,
				imageData.ImageHeight,
				LibGraphics::Texture::FORMAT::BGR24);

			for (auto& filter : imageFilters)
				glImage = filter->Apply(glImage);

			auto fut = glImage->Save(saveDirectory.String() + "/" + it->first, imageSaveThreadPool);
			saveImageFutures.push_back(std::move(fut));

			enhanceImageFutures.erase(it);

			break;
		}
	}

	for (size_t i = 0; i < saveImageFutures.size(); ++i)
	{
		if (saveImageFutures[i].IsReady())
		{
			completedImages++;
			saveImageFutures.erase(saveImageFutures.begin() + i);
		}
	}
}

bool ImageProcessingExecutor::Completed() const
{
	return completedImages == totalImages;
}

float ImageProcessingExecutor::PercentageCompleted() const
{
	return 100.0f * ((float)completedImages / (float)totalImages);
}