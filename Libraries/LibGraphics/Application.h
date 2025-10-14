#pragma once

#include <functional>

namespace LibGraphics
{
	class AppManager;

	class Application
	{
	public:
		void Run(const std::function<bool(float fps)>& loopFunc) const;
		~Application();
		void* GetHandle() const;
		
	private:
		Application(void* pWindow);
		void* window;
		friend class AppManager;
	};
}