#include "PhotoEditor.h"
#include "Events.h"

#include "LibCV/Image.h"
#include "LibCV/ImageFX.h"

#include <thread>
#include <iostream>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

PhotoEditor::PhotoEditor(const std::shared_ptr<PanelSharedData>& sharedData)
	: imageProcessor{ std::make_shared<ImageProcessor>() }
    , panelPhoto{ std::make_shared<UIPhoto>(sharedData, imageProcessor) }
    , panelEnhance{ std::make_shared<UIEnhance>(sharedData, imageProcessor) }
    , panelSettings{ std::make_shared<UISettings>(sharedData, imageProcessor) }
    , panelFilters{ std::make_shared<UIFilters>(sharedData, imageProcessor) }
    , panelThumbnails{ std::make_shared<UIThumbnails>(sharedData, imageProcessor) }
{
}

void PhotoEditor::Init()
{
    panelPhoto->Init();
	panelFilters->Init();
    panelEnhance->Init();
	panelSettings->Init();
    panelThumbnails->Init();
}

void PhotoEditor::Render(float dt)
{
    imageProcessor->Update();

    const auto contentSize = ImGui::GetContentRegionAvail() - ImGui::GetStyle().WindowPadding * 2.0f;
    if (ImGui::BeginChild("##IMAGE_SELECTION__LEFT_PANEL__", ImVec2{ contentSize.x * 0.20f, 0 }, ImGuiChildFlags_Border))
    {
        panelThumbnails->Render(dt);
    }
    ImGui::EndChild();
    ImGui::SameLine();
    if (ImGui::BeginChild("##__MIDDLE_PANEL__", ImVec2{ contentSize.x * 0.60f, 0 }, ImGuiChildFlags_Border))
    {
        panelPhoto->Render(dt);
    }
    ImGui::EndChild();
    ImGui::SameLine();
    if (ImGui::BeginChild("##__RIGHT_PANEL__", ImVec2{ contentSize.x * 0.20f, 0 }, ImGuiChildFlags_Border))
    {
        if (ImGui::BeginTabBar("##__RIGHT_TABS__"))
        {
            if (ImGui::BeginTabItem("Settings##__RIGHT_TABS__"))
            {
                ImGui::BeginDisabled(!imageProcessor->IsLoadImageCompleted());
                {
                    ImGui::BeginChild("##__RIGHT_TABS_SETTINGS__", ImVec2{ 0, 250.0f });
                    {
                        panelSettings->Render(dt);
                    }
                    ImGui::EndChild();

                    ImGui::BeginChild("##__RIGHT_TABS_FILTERS__", ImVec2{ 0, 175.0f });
                    {
                        panelFilters->Render(dt);
                    }
                    ImGui::EndChild();

                    ImGui::BeginChild("##__RIGHT_TABS_ENHANCE__", ImVec2{ 0, 0.0f }, ImGuiChildFlags_Border);
                    {
                        panelEnhance->Render(dt);
                    }
                    ImGui::EndChild();
                }
                ImGui::EndDisabled();

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
    }
    ImGui::EndChild();
}

