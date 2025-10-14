#include "Directory.h"

namespace LibCore 
{
	namespace Filesystem
	{
		Directory::Directory()
			: path{}
		{

		}

		Directory::Directory(const char* path)
			: path{ path }
		{

		}

		bool Directory::Exists() const
		{
			return std::filesystem::exists(path);
		}

		std::vector<File> Directory::ListFiles(bool recursive) const
		{
			std::vector<File> files;
			for (auto& p : ListPaths(recursive))
			{
				if (p.IsFile())
					files.emplace_back(File{ p.String().c_str() });
			}
			return files;
		}
		
		std::vector<Path> Directory::ListPaths(bool recursive) const
		{
			const auto options = std::filesystem::directory_options::skip_permission_denied;

			std::vector<Path> paths;
			if (recursive) 
			{
				for (auto& entry : std::filesystem::recursive_directory_iterator(path.string(), options)) 
				{
					paths.emplace_back(Path{ entry.path().string().c_str() });
				}
			}
			else 
			{
				for (auto& entry : std::filesystem::directory_iterator(path.string(), options))
				{
					paths.emplace_back(entry.path().string().c_str());
				}
			}

			return paths;
		}

		std::vector<Directory> Directory::ListDirectories(bool recursive) const
		{
			std::vector<Directory> directories;
			for (auto& p : ListPaths(recursive))
			{
				if (p.IsDirectory())
					directories.emplace_back(Directory{ p.String().c_str() });
			}
			return directories;
		}

		Directory Directory::Create(const char* path)
		{
			Directory dir{ path };
			std::filesystem::create_directories(path);
			return dir;
		}
	}
}