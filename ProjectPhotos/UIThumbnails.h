#pragma once

#include <chrono>

#include "GlobalDefs.h"
#include "ImageProcessor.h"

#include "LibCore/File.h"
#include "LibCore/ThreadPool.h"

class UIThumbnails : public UIHeader
{
public:
    UIThumbnails(const std::shared_ptr<PanelSharedData>& sharedData,
        const std::shared_ptr<ImageProcessor>& imageProcessor);
    ~UIThumbnails();
    void Init();
    void Render(float dt);

private:
    void Clear();
    void LoadImages();
    bool ShowImageToEdit();
    bool IsAcceptedImageFormat(const std::string& ext) const;

private:
    struct Thumbnail
    {
        bool ToEdit;
        std::string filename;
        LibCore::Async::Future<LibCV::ImageData> loadFuture;
        std::shared_ptr<LibCore::Async::CancelToken> cancelToken;
        std::shared_ptr<LibGraphics::Texture> thumbnailTexture;
    };

    std::shared_ptr<ImageProcessor> imageProcessor;
    std::map<std::string, std::shared_ptr<Thumbnail>> thumbnails;
    LibCore::Async::ThreadPool loadImagePool;

private: 
    // UI tools
    float thumbnailScale;
    std::chrono::high_resolution_clock::time_point currClickedTime;
    std::shared_ptr<Thumbnail> clickedThumbnail;
};