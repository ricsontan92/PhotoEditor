#pragma once

#include <functional>
#include "LibCore/Path.h"

namespace LibGraphics
{
	class AppManager;

	class Application
	{
	public:
		void Run(const std::function<bool(float fps)>& loopFunc) const;
		~Application();
		void* GetHandle() const;

		using DragDropFuncType = void(const std::vector<LibCore::Filesystem::Path>& paths);
		void RegisterDragDropCallback(const std::function<DragDropFuncType>& fnc);

	private:
		Application(void* pWindow);
		void* window;
		friend class AppManager;
		std::vector<std::function<DragDropFuncType>> dragdropCallbacks;
	};
}