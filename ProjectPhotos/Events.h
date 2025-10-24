#include "LibCore/EventSystem.h"
#include "LibCore/Path.h"

#include <vector>

namespace Event
{
	struct OverlayPopup : LibCore::Event::Event
	{
		OverlayPopup(const std::function<bool()>& overlayRender)
			:OverlayFunc{ overlayRender }
		{

		}

		std::function<bool()> OverlayFunc;
	};

	struct DragDropFiles : LibCore::Event::Event
	{
		DragDropFiles(const std::vector<LibCore::Filesystem::Path>& paths)
			: Paths{ paths }
		{}

		std::vector<LibCore::Filesystem::Path> Paths;
	};
}
