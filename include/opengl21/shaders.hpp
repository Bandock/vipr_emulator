#ifndef _SHADERS_HPP_
#define _SHADERS_HPP_

namespace VIPR_Emulator
{
	namespace Shader
	{
		const char *PrimaryVertexShader = R"(#version 120
#extension GL_ARB_explicit_attrib_location : require

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 tex;

out vec2 outTex;

void main()
{
	gl_Position = vec4(pos, 0.0f, 1.0f);
	outTex = tex;
})";
		const char *SecondaryFramebufferFragmentShader = R"(#version 120
#extension GL_ARB_explicit_attrib_location : require

in vec2 outTex;
out vec4 outColor;

uniform sampler2D SecondaryFramebufferTexture;

void main()
{
	outColor = texture2D(SecondaryFramebufferTexture, outTex);
})";

		const char *FontFragmentShader = R"(#version 120
#extension GL_ARB_explicit_attrib_location : require

uniform vec4 FontColor;
uniform bool FontFlag_Invert;

in vec2 outTex;
out vec4 outColor;

uniform sampler2D CurrentTexture;

void main()
{
	vec4 color_data = texture2D(CurrentTexture, outTex);
	outColor = (int(color_data.r * 256.0f) == (FontFlag_Invert ? 0 : 1)) ? FontColor : vec4(0.0f, 0.0f, 0.0f, 1.0f);
})";

		const char *MachineFragmentShader = R"(#version 120
#extension GL_ARB_explicit_attrib_location : require

in vec2 outTex;
out vec4 outColor;

uniform sampler2D DisplayTexture;

void main()
{
	vec4 color_data = texture2D(DisplayTexture, outTex);
	outColor = (int(color_data.r * 256.0f) == 1) ? vec4(1.0f, 1.0f, 1.0f, 1.0f) : vec4(0.0f, 0.0f, 0.0f, 1.0f);
})";
	}
}

#endif
