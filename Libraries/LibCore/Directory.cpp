#include "Directory.h"
#include <shobjidl.h>   // For IFileDialog
#include "StringUtils.h"

namespace LibCore 
{
	namespace Filesystem
	{
		Directory::Directory()
			: path{}
		{

		}

		Directory::Directory(const Path& path)
			: Directory{ path.String().c_str() }
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

		std::string Directory::String() const
		{
			return path.string();
		}

		Directory Directory::operator/(const std::string& lhs) const
		{
			return Directory{ (path / lhs).string().c_str() };
		}

		void Directory::Create() const
		{
			std::filesystem::create_directories(path);
		}

		Directory Directory::Create(const char* path)
		{
			Directory dir{ path };
			std::filesystem::create_directories(path);
			return dir;
		}

		Directory Directory::OpenDirectoryDialog()
		{
			IFileDialog* pFileDialog = nullptr;
			std::wstring folderPath;

			// Initialize COM
			if (SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)))
			{
				// Create FileOpenDialog instance
				if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
					IID_PPV_ARGS(&pFileDialog))))
				{
					DWORD options;
					pFileDialog->GetOptions(&options);
					// Add the FOS_PICKFOLDERS flag to choose folders instead of files
					pFileDialog->SetOptions(options | FOS_PICKFOLDERS);

					// Set dialog title
					pFileDialog->SetTitle(L"Select a Folder to Save Files");

					// Show dialog
					if (SUCCEEDED(pFileDialog->Show(NULL)))
					{
						IShellItem* pItem;
						if (SUCCEEDED(pFileDialog->GetResult(&pItem)))
						{
							PWSTR pszFolderPath = nullptr;
							if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFolderPath)))
							{
								folderPath = pszFolderPath;
								CoTaskMemFree(pszFolderPath);
							}
							pItem->Release();
						}
					}
					pFileDialog->Release();
				}
				CoUninitialize();
			}

			if (folderPath.empty())
			{
				throw std::exception{ "User cancelled" };
			}

			return Directory{ Utils::String::WStringToString(folderPath).c_str() };
		}
	}
}