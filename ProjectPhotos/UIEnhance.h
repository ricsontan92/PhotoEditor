#pragma once

#include "GlobalDefs.h"
#include "ImageProcessor.h"

class UIEnhance : public UIHeader
{
public:
	UIEnhance(
		const std::shared_ptr<PanelSharedData>& sharedData,
		const std::shared_ptr<ImageProcessor>& imageProcessor);
	void Init();
	void Render(float dt);

private:
	std::shared_ptr<ImageProcessor> imageProcessor;
};