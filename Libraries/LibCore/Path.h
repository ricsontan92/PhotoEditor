#pragma once

#include <filesystem>

namespace LibCore
{
	namespace Filesystem
	{
		class Path
		{
		public:
			Path();
			Path(const char* path);

			std::string String() const;
			std::string Filename() const;
			std::string Stem() const;
			std::string Extension() const;
			Path Parent() const;

			bool Exists() const;
			bool IsDirectory() const;
			bool IsFile() const;

		private:
			std::filesystem::path path;
		};
	}
}