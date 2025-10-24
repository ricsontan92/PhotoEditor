#include "UIThumbnails.h"
#include "Events.h"

#include "LibCore/File.h"
#include "LibCore/Directory.h"
#include "LibCore/StringUtils.h"

#include <set>
#include <iostream>

#define THUMBNAIL_MAX_SIZE 512

static std::set<std::string> ACCEPTED_IMAGE_TYPE{
    ".jpeg",
    ".jpg",
    ".png"
};

UIThumbnails::UIThumbnails(const std::shared_ptr<PanelSharedData>& sharedData,
    const std::shared_ptr<ImageProcessor>& imageProcessor)
    : UIHeader{ sharedData }
    , imageProcessor{ imageProcessor }
	, loadImagePool{ 2 }
	, thumbnailScale{ 1.0f }
	, currClickedTime{ std::chrono::high_resolution_clock::now() }
	, clickedThumbnail{ nullptr }
{
}

UIThumbnails::~UIThumbnails()
{
	Clear();
}

void UIThumbnails::Init()
{
    UISharedData->EvtSystem->AddListener<Event::DragDropFiles>([this](const Event::DragDropFiles& evt) {
		// clear all thumbails
		Clear();

		auto LoadThumbnailFunc = [this](const LibCore::Filesystem::File& file) {
			auto filename = file.FilePath().String();
			if (thumbnails.find(filename) == thumbnails.end())
			{
				thumbnails[filename] = std::make_shared<Thumbnail>();
				thumbnails[filename]->filename = file.FileName();
				thumbnails[filename]->cancelToken = std::make_shared<LibCore::Async::CancelToken>();
				thumbnails[filename]->loadFuture = loadImagePool.Enqueue(thumbnails[filename]->cancelToken, [file]() {
					auto image = LibCV::Image::Create(file.FilePath().String().c_str());
					if (image->Width() > THUMBNAIL_MAX_SIZE || image->Height() > THUMBNAIL_MAX_SIZE)
						image = image->Resize(image->Width() > image->Height() ? ((float)THUMBNAIL_MAX_SIZE / image->Width()) : ((float)THUMBNAIL_MAX_SIZE / image->Height()));
					return image->GetImageData();
				});
			}
		};

		// load thumbnails
        for (auto& path : evt.Paths)
        {
            if (path.IsDirectory())
            {
                LibCore::Filesystem::Directory directory{ path };
                for (auto& file : directory.ListFiles(false))
                {
                    if (IsAcceptedImageFormat(file.Extension()))
						LoadThumbnailFunc(file);
                }
            }
            else if (path.IsFile() && IsAcceptedImageFormat(path.Extension()))
				LoadThumbnailFunc(LibCore::Filesystem::File{ path.String().c_str() });
        }
    });
}

void UIThumbnails::Render(float dt)
{
	LoadImages();

	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
	ImGui::SliderFloat("##THUMBNAIL_SCALE_SLIDER", &thumbnailScale, 0.25f, 1.0f, "");
	ImGui::PopItemWidth();

	ImGui::BeginChild("##IMAGE_THUMBNAIL_CHILD");
	{
		const float extraSizes = ImGui::GetStyle().ItemSpacing.x + ImGui::GetStyle().ScrollbarSize;
		const float imageSize = ImGui::GetContentRegionAvail().x * thumbnailScale - extraSizes;
		float last_button_x2 = 0;

		const float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x - extraSizes;
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.y, ImGui::GetStyle().ItemSpacing.y));

		for (auto& thumbnail : thumbnails)
		{
			if (last_button_x2 != 0 && (last_button_x2 + imageSize) < window_visible_x2)
				ImGui::SameLine();

			ImGui::BeginGroup();
			{
				const float aspect = thumbnail.second->thumbnailTexture ? thumbnail.second->thumbnailTexture->GetAspect() : 1.0f;
				std::string filename = thumbnail.second->filename;

				ImGui::ImageButton(
					(filename + "##IMAGE_ID_" + std::to_string((long long)&thumbnail)).c_str(),
					(ImTextureID)(thumbnail.second->thumbnailTexture ? thumbnail.second->thumbnailTexture->GetHandler() : 0),
					ImVec2(imageSize, imageSize),
					aspect,
					ImVec2(0.0f, 0.0f),
					ImVec2(1.0f, 1.0f),
					ImVec4{ 0, 0, 0, 1 });

				// check if double clicked
				if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
				{
					// 2nd click
					if (clickedThumbnail == thumbnail.second)
					{
						auto timeElapsedSecs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - currClickedTime).count() / 1000.0f;
						if (timeElapsedSecs < ImGui::GetIO().MouseDoubleClickTime)
						{
							clickedThumbnail = nullptr;
							imageProcessor->LoadImage(LibCore::Filesystem::Path{ thumbnail.first.c_str() });
						}
					}
					else
					{
						// 1st click
						clickedThumbnail = thumbnail.second;
					}

					currClickedTime = std::chrono::high_resolution_clock::now();
				}

				float textWidth = ImGui::CalcTextSize(filename.c_str()).x;
				if (textWidth > imageSize)
				{
					while (textWidth > imageSize)
					{
						filename.pop_back();
						textWidth = ImGui::CalcTextSize((filename + "...").c_str()).x;
					}
					filename += "...";
				}

				bool hovered = false;
				{
					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0,0 });
					float indentWidth = std::max(0.f, 0.5f * (imageSize - textWidth));
					indentWidth == 0.0f ? void() : ImGui::Indent(indentWidth);
					ImGui::Text(filename.c_str());
					hovered = ImGui::IsItemHovered();
					ImGui::PopStyleVar();
				}
				if (hovered)
				{
					ImGui::BeginTooltip();
					ImGui::Text(thumbnail.second->filename.c_str());
					ImGui::EndTooltip();
				}
			}
			ImGui::EndGroup();

			last_button_x2 = ImGui::GetItemRectMax().x + ImGui::GetStyle().ItemSpacing.x;
		}

		ImGui::PopStyleVar();
	}
	ImGui::EndChild();
}

void UIThumbnails::Clear()
{
	for (auto& thumbnail : thumbnails)
		thumbnail.second->cancelToken->Cancel();

	for (auto& thumbnail : thumbnails)
	{
		if (thumbnail.second->loadFuture.Valid())
		{
			try
			{
				thumbnail.second->loadFuture.Get();
			}
			catch (const std::exception& e)
			{
				std::cout << "Load image failed: " << e.what() << std::endl;
			}
		}
	}

	thumbnails.clear();
}

void UIThumbnails::LoadImages()
{
	for (auto& thumbnail : thumbnails)
	{
		try
		{
			if (thumbnail.second->loadFuture.IsReady())
			{
				auto imageData = thumbnail.second->loadFuture.Get();
				thumbnail.second->thumbnailTexture = LibGraphics::Texture::CreateFromData(
					imageData.Pixels,
					imageData.ImageWidth,
					imageData.ImageHeight,
					LibGraphics::Texture::FORMAT::BGR24);
			}
		}
		catch (const std::exception& e)
		{
			std::cout << "Load image failed: " << e.what() << std::endl;
		}
	}
}

bool UIThumbnails::IsAcceptedImageFormat(const std::string& ext) const
{
    return ACCEPTED_IMAGE_TYPE.find(LibCore::Utils::String::ToLower(ext)) != ACCEPTED_IMAGE_TYPE.end();
}