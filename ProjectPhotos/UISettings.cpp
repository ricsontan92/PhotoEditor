#include "UISettings.h"

UISettings::UISettings(
	const std::shared_ptr<PanelSharedData>& sharedData,
	const std::shared_ptr<ImageProcessor>& imageProcessor)
	: UIHeader{ sharedData }
	, imageProcessor{ imageProcessor }
{
}

void UISettings::Init()
{
	imageProcessor->SetImageSettingDefaults();
}

void UISettings::SetSettingDragging(
	const ImageProcessor::IMAGE_SETTINGS setting,
	const std::string& name) const
{
	const auto range = imageProcessor->GetImageSettingRange(setting);
	const std::string settingsIdx = name + std::string("##IMAGE_SETTINGS");

	float value = 100.0f * (imageProcessor->GetImageSetting(setting) - range.x) / (range.y - range.x);
	if (ImGui::DragFloat(settingsIdx.c_str(), &value, 0.0f, 0.0f, 100.0f, "%.1f"))
	{
		imageProcessor->SetImageSetting(setting, range.x + (value / 100.0f) * (range.y - range.x));
	}
}

void UISettings::Render(float dt)
{
	ImGui::BeginChild("##SETTING_LIST", ImVec2{ 0, ImGui::GetContentRegionAvail().y }, ImGuiChildFlags_Border);
	{
		SetSettingDragging(ImageProcessor::IMAGE_SETTINGS::BRIGHTNESS,	"Brightness");
		SetSettingDragging(ImageProcessor::IMAGE_SETTINGS::CONSTRAST,	"Contrast");
		SetSettingDragging(ImageProcessor::IMAGE_SETTINGS::SHARPNESS,	"Sharpness");
		SetSettingDragging(ImageProcessor::IMAGE_SETTINGS::HUE,			"Hue");
		SetSettingDragging(ImageProcessor::IMAGE_SETTINGS::SATURATION,	"Saturation");
		SetSettingDragging(ImageProcessor::IMAGE_SETTINGS::LIGHTNESS,	"Lightness");
		SetSettingDragging(ImageProcessor::IMAGE_SETTINGS::TEMPERATURE, "Temperature");
		SetSettingDragging(ImageProcessor::IMAGE_SETTINGS::GAMMA,		"Gamma");

		if (ImGui::Button("Reset to Default", ImVec2{ ImGui::GetContentRegionAvail().x, 0 }))
		{
			imageProcessor->SetImageSettingDefaults();
		}
	}
	ImGui::EndChild();
}

