#include "PhotoEditor.h"

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
    , panelSettings{ std::make_shared<UISettings>(sharedData, imageProcessor) }
    , panelFilters{ std::make_shared<UIFilters>(sharedData, imageProcessor) }
{
}

void PhotoEditor::Init()
{
    panelPhoto->Init();
	panelFilters->Init();
	panelSettings->Init();


}

void PhotoEditor::Render(float dt)
{
    const auto contentSize = ImGui::GetContentRegionAvail() - ImGui::GetStyle().WindowPadding * 2.0f;
    if (ImGui::BeginChild("##__LEFT_PANEL__", ImVec2{ contentSize.x * 0.20f, 0 }, ImGuiChildFlags_Border))
    {
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

                ImGui::BeginChild("##__RIGHT_TABS_ENHANCE__", ImVec2{ 0, -30.0f }, ImGuiChildFlags_Border);
                {
                    ImGui::BeginDisabled(panelPhoto->IsLoading());
                    {

                        bool reloadImage = false;
                        reloadImage = ImGui::CheckboxFlags("Auto Brightness/Constrast", &imageProcessor->ImageFXFlags, LibCV::ImageFX::AUTO_BRIGHTNESS_CONTRAST) || reloadImage;
                        reloadImage = ImGui::CheckboxFlags("Auto Gamma", &imageProcessor->ImageFXFlags, LibCV::ImageFX::AUTO_GAMMA) || reloadImage;
                        reloadImage = ImGui::CheckboxFlags("Auto Color Temp", &imageProcessor->ImageFXFlags, LibCV::ImageFX::AUTO_COLOR_TEMP) || reloadImage;
                        reloadImage = ImGui::CheckboxFlags("Auto CLAHE", &imageProcessor->ImageFXFlags, LibCV::ImageFX::AUTO_CLAHE) || reloadImage;
                        reloadImage = ImGui::CheckboxFlags("Auto Details Enhance", &imageProcessor->ImageFXFlags, LibCV::ImageFX::AUTO_DETAIL_ENHANCE) || reloadImage;
                        reloadImage = ImGui::CheckboxFlags("Auto Denoise", &imageProcessor->ImageFXFlags, LibCV::ImageFX::AUTO_DENOISE) || reloadImage;

                        if (reloadImage)
                        {
                            panelPhoto->LoadPhoto(panelPhoto->GetPhotoPath().c_str());
                        }
                    }
                    ImGui::EndDisabled();
                }
                ImGui::EndChild();

                if (ImGui::Button("Apply To Images##PHOTOEDITOR_APPLY_IMAGES", ImVec2{ ImGui::GetContentRegionAvail().x, 0 } ))
                {

                }

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
    }
    ImGui::EndChild();
}

