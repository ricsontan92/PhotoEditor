#pragma once

#include "GlobalDefs.h"
#include "ImageProcessor.h"

#include "LibCV/Image.h"

#include <future>

class UIPhoto : public UIHeader
{
public:
    UIPhoto(
        const std::shared_ptr<PanelSharedData>& sharedData,
        const std::shared_ptr<ImageProcessor>& imageProcessor);
    void Init();
    void Render(float dt);
    bool IsLoading() const;
    void LoadPhoto(const char* imgPath);
    const std::string& GetPhotoPath() const;

private:
    std::future<void> loadPhotoFuture;

private:
    std::shared_ptr<ImageProcessor> imageProcessor;

private:
    float imageZoom;
    std::string imagePath;
    LibCore::Math::Vec2 imageOffset;

    float dragOffset;
    bool isImageDiffClicked;

private:
    std::shared_ptr<LibCV::Image> origCVImage, procCVImage;

private:
    std::shared_ptr<LibGraphics::Texture> origImage, enhancedImage;
};