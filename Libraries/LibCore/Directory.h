#pragma once

#include <filesystem>
#include "File.h"
#include "Path.h"

namespace LibCore
{
	namespace Filesystem
	{
		class Directory 
		{
		public:
			Directory();
			Directory(const Path& path);
			Directory(const char* path);
			bool Exists() const;
			std::vector<File> ListFiles(bool recursive) const;
			std::vector<Path> ListPaths(bool recursive) const;
			std::vector<Directory> ListDirectories(bool recursive) const;

			static Directory Create(const char* path);

		private:
			std::filesystem::path path;
		};
	}
}