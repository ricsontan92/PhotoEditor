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

private:
    std::shared_ptr<ImageProcessor> imageProcessor;
    std::shared_ptr<LibGraphics::Texture> currImage;
private:
    float imageZoom;
    std::string imagePath;
    LibCore::Math::Vec2 imageOffset;

    float dragOffset;
    bool isImageDiffClicked;
};