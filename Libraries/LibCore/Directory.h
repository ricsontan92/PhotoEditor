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
			std::string String() const;

			Directory operator/(const std::string& lhs) const;

			void Create() const;
			static Directory Create(const char* path);
			static Directory OpenDirectoryDialog();


		private:
			std::filesystem::path path;
		};
	}
}