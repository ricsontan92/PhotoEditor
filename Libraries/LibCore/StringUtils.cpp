#include "StringUtils.h"


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
	}
}