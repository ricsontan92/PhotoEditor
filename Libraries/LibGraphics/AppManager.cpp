#include "AppManager.h"
#include "Application.h"

#include "GL/glew.h"
#include "GLFW/glfw3.h" // Will drag system OpenGL headers

namespace LibGraphics
{
	std::shared_ptr<AppManager> AppManager::Create()
	{
		if (!glfwInit()) 
			return nullptr; // already initialised
		glfwSwapInterval(1); // Enable vsync

		return std::shared_ptr<AppManager>{ new AppManager{} };
	}

	AppManager::AppManager()
	{
		glfwSetErrorCallback([](int error, const char* description) {
			fprintf(stderr, "GLFW Error %d: %s\n", error, description);
		});
	}

	AppManager::~AppManager()
	{
		glfwTerminate();
	}

	std::shared_ptr<Application> AppManager::CreateApp(const char* title, int width, int height)
	{
		auto window = glfwCreateWindow(width, height, title, nullptr, nullptr);
		
		if (window == nullptr)
			return nullptr;

		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

		glfwWindowHint(GLFW_DEPTH_BITS, 24);
		glfwWindowHint(GLFW_RED_BITS, 8);
		glfwWindowHint(GLFW_GREEN_BITS, 8);
		glfwWindowHint(GLFW_BLUE_BITS, 8);

		glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

		glfwMakeContextCurrent(window);
		glfwSwapInterval(1); // Enable vsync

		glewExperimental = GL_TRUE;
		auto glErr = glewInit();
		if (glErr != GLEW_OK)
		{
			fprintf(stderr, "Error: %s\n", glewGetErrorString(glErr));
			return nullptr;
		}

		return std::shared_ptr<Application>{ new Application{ (void*)window } };
	}
}