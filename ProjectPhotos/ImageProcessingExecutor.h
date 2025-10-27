#pragma once

#include <memory>
#include "LibCore/Directory.h"
#include "ImageProcessor.h"

class ImageProcessingExecutor
{
public:
	static std::shared_ptr<ImageProcessingExecutor> Run(
		const std::shared_ptr<ImageProcessor>& processor,
		const std::vector<LibCore::Filesystem::File>& imageFiles,
		const LibCore::Filesystem::Directory& saveDirectory);
	~ImageProcessingExecutor();

	void Update();
	bool Completed() const;
	float PercentageCompleted() const;

private:
	ImageProcessingExecutor();
	ImageProcessingExecutor(const ImageProcessingExecutor&) = delete;
	ImageProcessingExecutor& operator=(const ImageProcessingExecutor&) = delete;

private:
	unsigned totalImages, completedImages;
	std::vector< LibCore::Async::Future<bool>> saveImageFutures;
	std::unordered_map<std::string, LibCore::Async::Future<LibCV::ImageData>> enhanceImageFutures;
	std::vector<std::shared_ptr<LibGraphics::TextureFilter>> imageFilters;
	LibCore::Async::ThreadPool imageEnhanceThreadPool, imageSaveThreadPool;
	LibCore::Filesystem::Directory saveDirectory;
};