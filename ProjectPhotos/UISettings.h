#pragma once

#include "GlobalDefs.h"
#include "ImageProcessor.h"

class UISettings : public UIHeader
{
public:
	UISettings( const std::shared_ptr<PanelSharedData>& sharedData, 
                const std::shared_ptr<ImageProcessor>& imageProcessor);
    void Init();
    void Render(float dt);

private:
    std::shared_ptr<ImageProcessor> imageProcessor;

private:
    void SetSettingDragging(const ImageProcessor::IMAGE_SETTINGS settings, const std::string& name) const;
};