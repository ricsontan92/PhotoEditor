#include "Mat4.h"

#include "glm/mat4x4.hpp"

namespace LibCore
{
	namespace Math
	{
		glm::mat4 Arr2Mat(const float m[16])
		{
			return glm::mat4{
				m[0], m[1], m[2], m[4],
				m[5], m[6], m[7], m[8],
				m[9], m[10], m[11], m[12],
				m[13], m[14], m[15], m[16],
			};
		}

		Mat4::Mat4()
			:mat{
				0,0,0,0,
				0,0,0,0,
				0,0,0,0,
				0,0,0,0 }
		{
		}
	}
}