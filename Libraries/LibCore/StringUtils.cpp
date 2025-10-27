#include "StringUtils.h"
#include <locale>
#include <codecvt>
#include <windows.h>

namespace LibCore
{
	namespace Utils
	{
		std::string String::ToLower(std::string str)
		{
			for (auto& c : str)
				c = std::tolower(c);
			return str;
		}

		std::string String::WStringToString(const std::wstring& wstr)
		{
			if (wstr.empty()) return {};
			int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), NULL, 0, NULL, NULL);
			std::string result(sizeNeeded, 0);
			WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), result.data(), sizeNeeded, NULL, NULL);
			return result;
		}
	}
}