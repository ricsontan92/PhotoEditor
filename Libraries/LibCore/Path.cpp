#include "Path.h"

namespace LibCore
{
	namespace Filesystem
	{
		Path::Path()
			: Path{ "" }
		{

		}

		Path::Path(const char* path)
			: path{ path }
		{

		}

		std::string Path::String() const
		{
			return path.string();
		}

		std::string Path::Filename() const
		{
			return path.filename().string();
		}

		std::string Path::Stem() const
		{
			return path.stem().string();
		}

		std::string Path::Extension() const
		{
			return path.extension().string();
		}

		Path Path::Parent() const
		{
			return Path{ path.parent_path().string().c_str() };
		}

		bool Path::Exists() const
		{
			return std::filesystem::exists(path);
		}

		bool Path::IsDirectory() const
		{
			return std::filesystem::is_directory(path);
		}

		bool Path::IsFile() const
		{
			return std::filesystem::is_regular_file(path);
		}
	}
}