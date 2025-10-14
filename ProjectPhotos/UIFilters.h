#pragma once

#include "GlobalDefs.h"
#include "ImageProcessor.h"

class UIFilters : public UIHeader
{
public:
	UIFilters(
		const std::shared_ptr<PanelSharedData>& sharedData,
		const std::shared_ptr<ImageProcessor>& imageProcessor);
    void Init();
    void Render(float dt);

private:
	std::shared_ptr<ImageProcessor> imageProcessor;

private:
	std::string filterSelectName;
	std::map<std::string, std::shared_ptr<LibGraphics::TextureFilter>> filtersMap;

private:
	struct UniformFloat
	{
		float DefaultValue;
		float MinValue, MaxValue;
	};
	std::map<std::string, std::map<std::string, UniformFloat>> shaderVars_Floats;

	struct UniformInt
	{
		int DefaultValue;
		int MinValue, MaxValue;
	};
	std::map<std::string, std::map<std::string, UniformInt>> shaderVars_Ints;

	struct UniformFloatVec2
	{
		LibCore::Math::Vec2 DefaultValue;
		float MinValue, MaxValue;
	};
	std::map<std::string, std::map<std::string, UniformFloatVec2>> shaderVars_FloatVec2s;

	struct UniformFloatVec3
	{
		LibCore::Math::Vec3 DefaultValue;
		float MinValue, MaxValue;
		bool IsColor;
	};
	std::map<std::string, std::map<std::string, UniformFloatVec3>> shaderVars_FloatVec3s;

	struct UniformFloatVec4
	{
		LibCore::Math::Vec4 DefaultValue;
		float MinValue, MaxValue;
		bool IsColor;
	};
	std::map<std::string, std::map<std::string, UniformFloatVec4>> shaderVars_FloatVec4s;
};