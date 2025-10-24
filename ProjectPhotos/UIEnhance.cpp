#include "UIEnhance.h"

UIEnhance::UIEnhance(
	const std::shared_ptr<PanelSharedData>& sharedData,
	const std::shared_ptr<ImageProcessor>& imageProcessor)
	: UIHeader{ sharedData }
	, imageProcessor{ imageProcessor }
{
}

void UIEnhance::Init()
{
}

void UIEnhance::Render(float dt)
{
    auto fxFlags = imageProcessor->GetFXFlags();
    ImGui::CheckboxFlags("Auto Brightness/Constrast",   &fxFlags, LibCV::ImageFX::AUTO_BRIGHTNESS_CONTRAST);
    ImGui::CheckboxFlags("Auto Gamma",                  &fxFlags, LibCV::ImageFX::AUTO_GAMMA);
    ImGui::CheckboxFlags("Auto Color Temp",             &fxFlags, LibCV::ImageFX::AUTO_COLOR_TEMP);
    ImGui::CheckboxFlags("Auto HSL",                    &fxFlags, LibCV::ImageFX::AUTO_HSL);
    ImGui::CheckboxFlags("Auto Sharpen",                &fxFlags, LibCV::ImageFX::AUTO_SHARPEN);
    //ImGui::CheckboxFlags("Auto CLAHE", &fxFlags, LibCV::ImageFX::AUTO_CLAHE);
    //ImGui::CheckboxFlags("Auto Details Enhance", &fxFlags, LibCV::ImageFX::AUTO_DETAIL_ENHANCE);
    //ImGui::CheckboxFlags("Auto Denoise", &fxFlags, LibCV::ImageFX::AUTO_DENOISE);

    imageProcessor->SetFXFlags(fxFlags);
}