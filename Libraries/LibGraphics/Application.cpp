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

	void Application::Run(const std::function<bool(float fps)>& loopFunc) const
	{
		float fps = 1.0f / 60.0f;

		glfwMakeContextCurrent((GLFWwindow*)window);
		while (!glfwWindowShouldClose((GLFWwindow*)window))
		{
			auto timeBeg = std::chrono::high_resolution_clock::now();
			{
				int display_w, display_h;
				glfwGetFramebufferSize((GLFWwindow*)window, &display_w, &display_h);
				glViewport(0, 0, display_w, display_h);
				glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
				glClear(GL_COLOR_BUFFER_BIT);

				// Poll and handle events (inputs, window resize, etc.)
				// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
				// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
				// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
				// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
				//if (glfwGetWindowAttrib((GLFWwindow*)window, GLFW_ICONIFIED) != 0)
				//{
				//	std::this_thread::sleep_for(std::chrono::milliseconds{ 10 });
				//	continue;
				//}

				glfwSetWindowShouldClose((GLFWwindow*)window, loopFunc(fps) ? 1 : 0);

				glfwPollEvents();
				glfwSwapBuffers((GLFWwindow*)window);
			}
			fps = fps * 0.9f + 0.1f * (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - timeBeg).count() / 1000.0f);
		}
	}
}