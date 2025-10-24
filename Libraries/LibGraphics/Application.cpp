#include "Application.h"

#include "GLFW/glfw3.h" // Will drag system OpenGL headers

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

#include <chrono>
#include <thread>

namespace LibGraphics
{
	Application::Application(void * pWindow)
		: window{ pWindow }
	{
		glfwSetWindowUserPointer((GLFWwindow*)window, this);
		glfwSetDropCallback((GLFWwindow*)window, [](GLFWwindow* window, int count, const char** paths){
			auto pApplication = (Application*)glfwGetWindowUserPointer(window);
			
			std::vector<LibCore::Filesystem::Path> filePaths;
			for (int i = 0; i < count; i++)
				filePaths.emplace_back(LibCore::Filesystem::Path{ paths[i] });

			for (auto& fnc : pApplication->dragdropCallbacks)
				fnc(filePaths);
		});
	}

	Application::~Application()
	{
		glfwDestroyWindow((GLFWwindow*)window);
		glfwMakeContextCurrent(nullptr);
		window = nullptr;
	}

	void* Application::GetHandle() const
	{
		return window;
	}

	void Application::RegisterDragDropCallback(const std::function<DragDropFuncType>& fnc)
	{
		dragdropCallbacks.push_back(fnc);
	}

	void Application::Run(const std::function<bool(float fps)>& loopFunc) const
	{
		glfwMakeContextCurrent((GLFWwindow*)window);

		const double targetFPS = 60.0f;

		float fps = 1.0f / static_cast<float>(targetFPS);
		while (!glfwWindowShouldClose((GLFWwindow*)window))
		{
			auto timeBeg = std::chrono::high_resolution_clock::now();
			{
				int display_w, display_h;
				glfwGetFramebufferSize((GLFWwindow*)window, &display_w, &display_h);
				glViewport(0, 0, display_w, display_h);
				glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
				glClear(GL_COLOR_BUFFER_BIT);

				glfwSetWindowShouldClose((GLFWwindow*)window, loopFunc(fps) ? 1 : 0);

				glfwPollEvents();
				glfwSwapBuffers((GLFWwindow*)window);
			}
			fps = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - timeBeg).count() / 1000.0f;
		}
	}
}