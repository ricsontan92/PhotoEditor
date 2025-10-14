#include "Vec4.h"

namespace LibCore
{
	namespace Math
	{
		Vec4::Vec4() : x{ 0 } , y{ 0 }
		{

		}

		Vec4::Vec4(float s) : x{ s }, y{ s }, z{ s }, w{ s }
		{

		}

		Vec4::Vec4(float x, float y, float z, float w) : x{ x }, y{ y }, z{ z }, w{ w }
		{

		}

		Vec4 Vec4::operator/(const float& rhs) const
		{
			return Vec4{ x / rhs, y / rhs, z / rhs, w / rhs };
		}

		Vec4 Vec4::operator*(const float& rhs) const
		{
			return Vec4{ x * rhs, y * rhs, z * rhs, w * rhs };
		}
	}
}
