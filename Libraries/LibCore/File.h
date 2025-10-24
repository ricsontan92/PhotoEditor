#pragma once

#include <filesystem>
#include "Path.h"

namespace LibCore
{
	namespace Filesystem
	{
		class File 
		{
		public:
			File();
			File(const char* path);

			std::string Extension() const;
			std::string FileName() const;
			Path FilePath() const;
			bool Exists() const;
			size_t Size() const;
			bool Remove() const;
			bool Copy(const Path& to, bool overwrite = true);

			std::string ReadText() const;
			std::vector<uint8_t> ReadBinary() const;

			bool Write(const std::string& content) const;
			bool Write(const std::vector<uint8_t>& data) const;
			bool Write(const uint8_t* data, size_t size) const;

		private:
			std::filesystem::path path;
		};
	}
}