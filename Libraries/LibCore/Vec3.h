#pragma once

namespace LibCore
{
	namespace Math
	{
		struct Vec3
		{
			Vec3();
			Vec3(float s);
			Vec3(float x, float y, float z);

			float x, y, z;
		};
	}
}