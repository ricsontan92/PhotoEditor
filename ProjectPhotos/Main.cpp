#include <iostream>
#include <sstream>
#include <thread>

#include "PhotoEditor.h"

#include "LibCV/Image.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <future>
#include <filesystem>

int main()
{
    //_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    auto appManager = LibGraphics::AppManager::Create();
    {
        auto sharedData = std::make_shared<PanelSharedData>();
        sharedData->Application = appManager->CreateApp("FoodieLite", 1280, 720);
        {
            // Setup Dear ImGui context
            IMGUI_CHECKVERSION();
            auto imguiCtx = ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO(); (void)io;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

            // Setup Dear ImGui style
            ImGui::StyleColorsLight();

            const float roundings = 7.5f;
            ImGui::GetStyle().ChildRounding = roundings;
            ImGui::GetStyle().FrameRounding = roundings;
            ImGui::GetStyle().GrabRounding = roundings;
            ImGui::GetStyle().PopupRounding = roundings;
            ImGui::GetStyle().ScrollbarRounding = roundings;
            ImGui::GetStyle().TabRounding = roundings;

            // Setup Platform/Renderer backends
            ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)sharedData->Application->GetHandle(), true);
            ImGui_ImplOpenGL3_Init(nullptr);

            // Load Fonts
            ImFont* font = io.Fonts->AddFontFromFileTTF("assets/fonts/DroidSans.ttf", 16.50f);
            IM_ASSERT(font != nullptr);

            // declare Pages
            auto photoEditor = std::make_shared< PhotoEditor>(sharedData);

            // initialise
            photoEditor->Init();

#if 0
            {
                std::vector<std::future<void>> futures;
                for (const auto& entry : std::filesystem::directory_iterator("assets/images/to_edit/unedited/")) 
                {
                    auto fnc = [entry]() {
                        auto filename = "assets/images/to_edit/edited/" / entry.path().filename();
                        auto testCV = LibCV::Image::Create(entry.path().string())->AutoEnhance();
                        testCV->Write(filename.string().c_str());
                    };
                    futures.emplace_back(std::move(std::async(fnc)));

                    while (futures.size() >= std::thread::hardware_concurrency() / 2) 
                    {
                        for (size_t i = 0; i < futures.size(); ++i) 
                        {
                            if (futures[i].wait_for(std::chrono::seconds{ 0 }) == std::future_status::ready)
                            {
                                futures.erase(futures.begin() + i);
                            }
                        }
                    }

                    std::this_thread::sleep_for(std::chrono::milliseconds{ 16 });
                }
            }
#endif

            sharedData->Application->Run([&](float dt) {
                // Start the Dear ImGui frame
                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();

                ImGui::SetNextWindowPos(ImVec2{ 0, 0 });
                ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

                if (ImGui::Begin("##__MAIN_PANEL__", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_MenuBar))
                {
                    if (ImGui::BeginMenuBar())
                    {
                        if (ImGui::BeginMenu("  File  "))
                        {
                            if (ImGui::MenuItem("Import", "")) {}
                            if (ImGui::MenuItem("Export", "")) {}
                            if (ImGui::MenuItem("Save", "")) {}
                            if (ImGui::MenuItem("Undo", "")) {}
                            if (ImGui::MenuItem("Redo", "")) {}
                            ImGui::EndMenu();
                        }

                        if (ImGui::BeginMenu("  View  "))
                        {
                            if (ImGui::MenuItem("Zoom in", "")) {}
                            if (ImGui::MenuItem("Zoom out", "")) {}
                            if (ImGui::MenuItem("Fit to screen", "")) {}
                            ImGui::EndMenu();
                        }
                        ImGui::EndMenuBar();
                    }

                    photoEditor->Render(dt);
                }
                ImGui::End();

                std::string textFPS = (std::stringstream{} << std::floorf(1.0f / dt) << " FPS").str();
                auto textSize = ImGui::CalcTextSize(textFPS.c_str());
                ImGui::GetForegroundDrawList()->AddText(ImVec2{ ImGui::GetIO().DisplaySize.x - textSize.x - 2.5f, 2.5f }, 0xff0000ff, textFPS.c_str());
                
                // Rendering
                ImGui::Render();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

                ImGui::ResetMouseDragDelta(0);
                ImGui::ResetMouseDragDelta(1);
                ImGui::ResetMouseDragDelta(2);

                return false;
            });

            // Cleanup
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        }
    }
    return 0;
}

