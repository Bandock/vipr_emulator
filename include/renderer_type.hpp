#ifndef _RENDERER_TYPE_HPP_
#define _RENDERER_TYPE_HPP_

#include <array>

namespace VIPR_Emulator
{
	enum class RendererType
	{
		OpenGL_21,
		OpenGL_30,
		OpenGLES_2,
		OpenGLES_3
	};

	template <RendererType Type, RendererType... Args>
	consteval bool CheckCompatibleRendererType()
	{
		std::array<RendererType, sizeof...(Args)> CompatibleTypes = { Args... };
		for (size_t i = 0; i < CompatibleTypes.size(); ++i)
		{
			if (Type == CompatibleTypes[i])
			{
				return true;
			}
		}
		return false;
	}
}

#endif
