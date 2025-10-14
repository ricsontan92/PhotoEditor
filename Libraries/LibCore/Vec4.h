#pragma once

namespace LibCore
{
	namespace Math
	{
		struct Vec4
		{
			Vec4();
			Vec4(float s);
			Vec4(float x, float y, float z, float w);

			Vec4 operator/(const float& rhs) const;
			Vec4 operator*(const float& rhs) const;

			float x, y, z, w;
		};
	}
}