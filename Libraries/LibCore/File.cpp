#include "File.h"
#include <fstream>

namespace LibCore
{
	namespace Filesystem
	{
		File::File()
			: File{""}
		{

		}

		File::File(const char* path)
			: path{ path }
		{

		}

		std::string File::Extension() const
		{
			return path.extension().string();
		}

		std::string File::FileName() const
		{
			return path.filename().string();
		}

		Path File::FilePath() const
		{
			return Path{ path.string().c_str() };
		}

		bool File::Exists() const
		{
			return std::filesystem::exists(path);
		}

		size_t File::Size() const
		{
			return static_cast<size_t>(std::filesystem::file_size(path));
		}

		bool File::Remove() const
		{
			if (Exists())
			{
				return std::filesystem::remove(path);
			}
			return false;
		}

		bool File::Copy(const Path& to, bool overwrite)
		{
			if (!Exists()) 
				return false;

			auto options = overwrite
				? std::filesystem::copy_options::overwrite_existing
				: std::filesystem::copy_options::none;

			try {
				std::filesystem::copy_file(path.string(), to.String(), options);
				return true;
			}
			catch (...) {
				return false;
			}
		}

		std::string File::ReadText() const
		{
			if (!Exists()) 
				return {};

			std::ifstream file{ path.string(), std::ios::in };
			if (!file.is_open()) 
				return {};

			return std::string(
				std::istreambuf_iterator<char>(file),
				std::istreambuf_iterator<char>()
			);
		}

		std::vector<uint8_t> File::ReadBinary() const
		{
			if (!Exists())
				return {};

			std::ifstream file{ path.string(), std::ios::binary | std::ios::ate };
			if (!file.is_open()) 
				return {};

			auto size = file.tellg();
			std::vector<uint8_t> buffer(static_cast<size_t>(size));
			file.seekg(0, std::ios::beg);
			file.read(reinterpret_cast<char*>(buffer.data()), size);
			return buffer;
		}

		bool File::Write(const std::string& content) const
		{
			return Write((const uint8_t*)content.data(), content.size());
		}

		bool File::Write(const std::vector<uint8_t>& content) const
		{
			return Write(content.data(), content.size());
		}

		bool File::Write(const uint8_t* data, size_t size) const
		{
			std::ofstream file{ path.string(), std::ios::out | std::ios::trunc };
			if (!file.is_open()) return false;

			file.write((const char*)data, static_cast<std::streamsize>(size));
			return true;
		}
	}
}