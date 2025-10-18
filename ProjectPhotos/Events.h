#include "LibCore/EventSystem.h"
#include "LibCore/Path.h"

#include <vector>

namespace Event
{
	struct DragDropFiles : LibCore::Event::Event
	{
		DragDropFiles(const std::vector<LibCore::Filesystem::Path>& paths)
			: Paths{ paths }
		{}

		std::vector<LibCore::Filesystem::Path> Paths;
	};
}
