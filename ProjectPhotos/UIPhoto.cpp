#include "UIPhoto.h"
#include "LibCV/ImageFX.h"

#include <iostream>
#include <algorithm>

UIPhoto::UIPhoto(
	const std::shared_ptr<PanelSharedData>& sharedData,
	const std::shared_ptr<ImageProcessor>& imageProcessor)
	: UIHeader{ sharedData }
	, imageProcessor{ imageProcessor }
    , imageZoom{ 0.0f }
    , imageOffset{ 0, 0 }
    , isImageDiffClicked{ false }
    , dragOffset{ 0 }
{
}

void UIPhoto::Init()
{
    imageProcessor->LoadImage("assets/images/DSC_0857.jpg");
}

void UIPhoto::Render(float dt)
{
    if (isImageDiffClicked && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        isImageDiffClicked = false;

    const bool isLoadingImage = imageProcessor->IsLoadImageCompleted();

    // fit to content size
    currImage = isLoadingImage ? imageProcessor->GetProcessedGLImage() : nullptr;
    if (currImage)
    {
        float aspect = currImage ? currImage->GetAspect() : 1.0f;
        auto regionSize = LibCore::Math::Vec2{ ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y };
        auto imageSize = LibCore::Math::Vec2{ regionSize.x, regionSize.x / aspect };
        if (imageSize.y > regionSize.y)
        {
            imageSize.y = regionSize.y - 40.0f;
            imageSize.x = imageSize.y * aspect;
        }
        else
        {
            imageSize.x = regionSize.x - 20.0f;
        }

        const ImVec2 imagePos = (ImGui::GetWindowSize() - ImVec2{ imageSize.x, imageSize.y }) * 0.5f;
        {
            ImGui::SetCursorPos(imagePos);
            ImGui::Image(
                currImage ? currImage->GetHandler() : 0,
                ImVec2{ imageSize.x, imageSize.y },
                ImVec2{ 0.0f + imageZoom + imageOffset.x, 0.0f + imageZoom + imageOffset.y },
                ImVec2{ 1.0f - imageZoom + imageOffset.x, 1.0f - imageZoom + imageOffset.y },
                ImVec4{ 1,1,1,1 },
                ImVec4{ 0,0,0,0 });
        }
        const bool isImageHovered = ImGui::IsItemHovered();
        const ImVec2 imageBeg = ImGui::GetItemRectMin();
        const ImVec2 imageEnd = ImGui::GetItemRectMax();
        // image dragger
        {
            const float DRAG_RADIUS = 8.0f;
            const ImVec2 dragBeg = imageBeg - ImVec2{ 0.0f, DRAG_RADIUS + 2.0f };

            isImageDiffClicked = isImageDiffClicked || 
                (ImGui::IsMouseHoveringCircle(dragBeg + ImVec2{ dragOffset, 0 }, DRAG_RADIUS) && ImGui::IsMouseClicked(ImGuiMouseButton_Left));

            if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && isImageDiffClicked)
                dragOffset = ImGui::GetMousePos().x - dragBeg.x;
            dragOffset = std::clamp(dragOffset, 0.0f, imageSize.x);

            ImGui::GetWindowDrawList()->AddLine(dragBeg, ImVec2{ imageEnd.x, dragBeg.y }, 0xffb0a0af, 2.0f);

            ImGui::GetWindowDrawList()->AddLine(dragBeg + ImVec2{ dragOffset, 0.0f }, ImVec2{ dragBeg.x + dragOffset, imageEnd.y }, 0xff0000af, 2.0f);
            ImGui::GetWindowDrawList()->AddCircle(dragBeg + ImVec2{ dragOffset, 0.0f }, DRAG_RADIUS + 1.0f, 0xffa0bcff);
            ImGui::GetWindowDrawList()->AddCircleFilled(dragBeg + ImVec2{ dragOffset, 0.0f }, DRAG_RADIUS, isImageDiffClicked ? 0xab0b0bff : 0xca0b70ff);
        }

        if (isImageHovered)
        {
            if (ImGui::GetIO().MouseWheel)
            {
                imageZoom = std::min(0.485f, std::max(0.0f, imageZoom + ImGui::GetIO().MouseWheel * dt));
            }

            if (ImGui::IsMouseDown(ImGuiMouseButton_Middle))
            {
                ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Middle, 0.0f);
                delta = ImVec2{ delta.x / imageSize.x, delta.y / imageSize.y };
                imageOffset.x -= delta.x * (1.0f - imageZoom * 2.0f);
                imageOffset.y -= delta.y * (1.0f - imageZoom * 2.0f);
            }
        }

        if (imageZoom + imageOffset.x < 0.0f) imageOffset.x = -imageZoom;
        if (imageOffset.x - imageZoom > 0.0f) imageOffset.x = +imageZoom;
        if (imageZoom + imageOffset.y < 0.0f) imageOffset.y = -imageZoom;
        if (imageOffset.y - imageZoom > 0.0f) imageOffset.y = +imageZoom;

        const float imgRatio = 1.0f / (1.0f - imageZoom * 2.0f);
        const ImVec2 newImgSiz{ imageSize.x * imgRatio, imageSize.y * imgRatio };
        const ImVec2 newImgBeg{ imageBeg.x - newImgSiz.x * (imageZoom + imageOffset.x), imageBeg.y - newImgSiz.y * (imageZoom + imageOffset.y) };

        auto normCursorPos = (ImGui::GetMousePos() - newImgBeg) / newImgSiz;

        if (isImageHovered)
        {
            auto pixelLoc = ImVec2{
                std::roundf(imageProcessor->GetImageWidth() * normCursorPos.x),
                std::roundf(imageProcessor->GetImageHeight() * normCursorPos.y) };

            auto pixelColor = currImage->GetPixel((int)(currImage->GetWidth() * normCursorPos.x), (int)(currImage->GetHeight() * normCursorPos.y));
            ImGui::SetCursorPosX(imageBeg.x - ImGui::GetWindowPos().x);

            ImGui::Text("Cursor: (%d, %d), Color: (%.2f, %.2f, %.2f, %.2f)", (int)pixelLoc.x, (int)pixelLoc.y, pixelColor.x, pixelColor.y, pixelColor.z, pixelColor.w);
            ImGui::GetWindowDrawList()->AddRectFilledOutlined(
                ImVec2{ 10.f + ImGui::GetItemRectMax().x, ImGui::GetItemRectMin().y},
                ImVec2{ 10.f + ImGui::GetItemRectMax().x + ImGui::GetItemRectSize().y , ImGui::GetItemRectMax().y },
                ImColor(pixelColor.x, pixelColor.y, pixelColor.z, pixelColor.w),
                ImColor(0.f, 0.f, 0.f, 1.0f));
        }

        const float visibleWidth = imageSize.x * 0.25f; // show 20% of processed image
        ImGui::PushClipRect(
            imageBeg,
            imageBeg + ImVec2{ (imageEnd.x - imageBeg.x) * (dragOffset / imageSize.x), imageEnd.y - imageBeg.y},
            true );

        {
            auto origImage = imageProcessor->GetOriginalGLImage();
            ImGui::SetCursorPos(imagePos);
            ImGui::Image(
                origImage ? origImage->GetHandler() : 0,
                ImVec2{ imageSize.x, imageSize.y },
                ImVec2{ 0.0f + imageZoom + imageOffset.x, 0.0f + imageZoom + imageOffset.y },
                ImVec2{ 1.0f - imageZoom + imageOffset.x, 1.0f - imageZoom + imageOffset.y },
                ImVec4{ 1,1,1,1 },
                ImVec4{ 0,0,0,0 });

            static const char* ORIGINAL_TEXT = " Original ";
            ImVec2 textSize = ImGui::CalcTextSize(ORIGINAL_TEXT);
            ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMin() + textSize, 0xff00ffff);

            ImGui::SetCursorPos(imagePos);
            ImGui::Text(ORIGINAL_TEXT);
        }
        ImGui::PopClipRect();
    }
    else // loading
    {
        float CIRC_RADIUS = 40.0f;
        ImGui::SetCursorPos(ImGui::GetWindowSize() * 0.5f - ImVec2{ CIRC_RADIUS, CIRC_RADIUS });
        ImGui::LoadingIndicatorCircle(
            "Loading...",
            CIRC_RADIUS,
            ImVec4(1, 1, 1, 1),
            ImVec4(0, 0, 1, 1),
            14,
            5.f);
    }
}