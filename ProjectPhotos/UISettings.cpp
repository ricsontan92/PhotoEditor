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
	imageProcessor->ImageSettings.push_back(BrightnessFilter = LibGraphics::TextureFilter::CreateFromShader(LibGraphics::BRIGHTNESS_SHADER));
	imageProcessor->ImageSettings.push_back(ContrastFilter = LibGraphics::TextureFilter::CreateFromShader(LibGraphics::CONTRAST_SHADER));
	imageProcessor->ImageSettings.push_back(SharpnessFilter = LibGraphics::TextureFilter::CreateFromShader(LibGraphics::SHARPEN_SHADER));
	imageProcessor->ImageSettings.push_back(HSLFilter = LibGraphics::TextureFilter::CreateFromShader(LibGraphics::HSL_ADJUSTMENT_SHADER));
	imageProcessor->ImageSettings.push_back(TemperatureFilter = LibGraphics::TextureFilter::CreateFromShader(LibGraphics::TEMPERATURE_SHADER));
	imageProcessor->ImageSettings.push_back(GammaFilter = LibGraphics::TextureFilter::CreateFromShader(LibGraphics::GAMMA_SHADER));

	SetDefaultValues();
}

void UISettings::SetSettingDragging(
	const std::shared_ptr<LibGraphics::TextureFilter>& settingFilter,
	const char* settingName,	
	float minValue, float maxValue) const
{
	float value = 0;
	if (settingFilter->GetFloat(settingName, value))
	{
		std::string settingsIdx = std::string{ settingName }.substr(1) + std::string("##IMAGE_SETTINGS");

		value = 100.0f * (value - minValue) / (maxValue - minValue);
		if (ImGui::DragFloat(settingsIdx.c_str(), &value, 0.0f, 0.0f, 100.0f, "%.1f"))
		{
			settingFilter->SetFloat(settingName, minValue + (value / 100.0f) * (maxValue - minValue));
		}
	}
}

void UISettings::SetDefaultValues()
{
	// set default values
	BrightnessFilter ? BrightnessFilter->SetFloat("uBrightness", 0.0f) : void();
	ContrastFilter ? ContrastFilter->SetFloat("uContrast", 1.0f) : void();
	SharpnessFilter ? SharpnessFilter->SetFloat("uSharpness", 0.0f) : void();
	HSLFilter ? HSLFilter->SetFloat("uHue", 0.0f) : void();
	HSLFilter ? HSLFilter->SetFloat("uSaturation", 1.0f) : void();
	HSLFilter ? HSLFilter->SetFloat("uLightness", 0.5f) : void();
	TemperatureFilter ? TemperatureFilter->SetFloat("uTemperature", 0.0f) : void();
	GammaFilter ? GammaFilter->SetFloat("uGamma", 1.0f) : void();
}

void UISettings::Render(float dt)
{
	ImGui::BeginChild("##SETTING_LIST", ImVec2{ 0, ImGui::GetContentRegionAvail().y }, ImGuiChildFlags_Border);
	{
		SetSettingDragging(BrightnessFilter,	"uBrightness",	-1.0f,		+1.0f);
		SetSettingDragging(ContrastFilter,		"uContrast",	+0.0f,		+2.0f);
		SetSettingDragging(SharpnessFilter,		"uSharpness",	-2.0f,		+2.0f);
		SetSettingDragging(HSLFilter,			"uHue",			-360.0f,	+360.0f);
		SetSettingDragging(HSLFilter,			"uSaturation",	+0.0f,		+2.0f);
		SetSettingDragging(HSLFilter,			"uLightness",	+0.0f,		+1.0f);
		SetSettingDragging(TemperatureFilter,	"uTemperature", -1.0f,		+1.0f);
		SetSettingDragging(GammaFilter,			"uGamma",		0.0f,		+2.0f);

		if (ImGui::Button("Reset to Default", ImVec2{ ImGui::GetContentRegionAvail().x, 0 }))
		{
			SetDefaultValues();
		}
	}
	ImGui::EndChild();
}

