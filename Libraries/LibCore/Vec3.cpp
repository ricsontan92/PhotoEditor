#include "Vec3.h"

namespace LibCore
{
	namespace Math
	{
		Vec3::Vec3() : x{ 0 } , y{ 0 }
		{

		}

		Vec3::Vec3(float s) : x{ s }, y{ s }, z{ s }
		{

		}

		Vec3::Vec3(float x, float y, float z) : x{ x }, y{ y }, z{ z }
		{

		}
	}
}
