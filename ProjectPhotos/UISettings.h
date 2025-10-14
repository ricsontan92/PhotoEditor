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
    void SetSettingDragging(
        const std::shared_ptr<LibGraphics::TextureFilter>& settingFilter,
        const char* settingName,
        float minValue, float maxValue
    ) const;
    void SetDefaultValues();

    std::shared_ptr<LibGraphics::TextureFilter> SharpnessFilter;
    std::shared_ptr<LibGraphics::TextureFilter> ContrastFilter;
    std::shared_ptr<LibGraphics::TextureFilter> BrightnessFilter;
    std::shared_ptr<LibGraphics::TextureFilter> HSLFilter;
    std::shared_ptr<LibGraphics::TextureFilter> TemperatureFilter;
    std::shared_ptr<LibGraphics::TextureFilter> GammaFilter;
};