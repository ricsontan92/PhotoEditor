#pragma once

#include <map>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "LibCV/Image.h"

#include "LibGraphics/Shader.h"
#include "LibGraphics/Texture.h"
#include "LibGraphics/AppManager.h"
#include "LibGraphics/Application.h"
#include "LibGraphics/TextureFilter.h"
#include "LibGraphics/DefaultShaders.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "LibCore/ThreadPool.h"
#include "LibCore/EventSystem.h"

struct PanelSharedData
{
	std::shared_ptr<LibGraphics::Texture> GLTexture;
	std::shared_ptr<LibGraphics::Application> Application;
	std::shared_ptr<LibCore::Event::EventSystem> EvtSystem;
	std::shared_ptr<LibCore::Async::ThreadPool> ThreadPool;
};

class UIHeader
{
public:
	virtual ~UIHeader() = default;
	virtual void Init() = 0;
	virtual void Render(float dt) = 0;

protected:
	UIHeader(const std::shared_ptr<PanelSharedData>& sharedData) : UISharedData{ sharedData } {}
	std::shared_ptr<PanelSharedData> UISharedData;
};