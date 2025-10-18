#pragma once

#include "UIPhoto.h"
#include "UIFilters.h"
#include "UIEnhance.h"
#include "UISettings.h"

#include <string>
#include <future>

class PhotoEditor
{
public:
	PhotoEditor(const std::shared_ptr< PanelSharedData>& sharedData);
	void Init();
	void Render(float dt);

private:
	std::shared_ptr<ImageProcessor> imageProcessor;
	std::shared_ptr<UIPhoto> panelPhoto;
	std::shared_ptr<UIFilters> panelFilters;
	std::shared_ptr<UIEnhance> panelEnhance;
	std::shared_ptr<UISettings> panelSettings;
};