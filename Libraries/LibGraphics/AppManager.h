#pragma once
#include <memory>

namespace LibGraphics
{
	class Application;

	class AppManager
	{
	public:
		static std::shared_ptr<AppManager> Create();
		~AppManager();

		std::shared_ptr<Application> CreateApp(const char* title, int width, int height);

	private:
		AppManager();
	};
}