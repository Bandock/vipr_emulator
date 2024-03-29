#ifndef _SHADERS_HPP_
#define _SHADERS_HPP_

namespace VIPR_Emulator
{
	namespace Shader
	{
		const char *PrimaryVertexShader = R"(#version 300 es

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 tex;

out vec2 outTex;

void main()
{
	gl_Position = vec4(pos, 0.0f, 1.0f);
	outTex = tex;
})";
		const char *SecondaryFramebufferFragmentShader = R"(#version 300 es

precision highp float;
precision highp int;
precision highp sampler2D;

in vec2 outTex;
out vec4 outColor;

uniform sampler2D SecondaryFramebufferTexture;

void main()
{
	ivec2 texDim = textureSize(SecondaryFramebufferTexture, 0);
	outColor = texelFetch(SecondaryFramebufferTexture, ivec2(int(outTex.x * float(texDim.x)), int(outTex.y * float(texDim.y))), 0);
})";
		const char *FontFragmentShader = R"(#version 300 es

precision highp float;
precision highp int;
precision highp usampler2D;

layout(std140) uniform FontControl
{
	vec4 FontColor;
	uint FontFlags;
};

in vec2 outTex;
layout(location = 0) out vec4 outColor;

uniform usampler2D CurrentTexture;

void main()
{
	ivec2 texDim = textureSize(CurrentTexture, 0);
	uvec4 color_data = texelFetch(CurrentTexture, ivec2(int(outTex.x * float(texDim.x)), int(outTex.y * float(texDim.y))), 0);
	outColor = (color_data.r == ((FontFlags & uint(0x01)) == uint(0x01) ? uint(0) : uint(1))) ? FontColor : vec4(0.0f, 0.0f, 0.0f, 1.0f);
})";
		const char *MachineFragmentShader = R"(#version 300 es

precision highp float;
precision highp int;
precision highp usampler2D;

in vec2 outTex;
out vec4 outColor;

uniform usampler2D DisplayTexture;

void main()
{
	ivec2 texDim = textureSize(DisplayTexture, 0);
	uvec4 color_data = texelFetch(DisplayTexture, ivec2(int(outTex.x * float(texDim.x)), int(outTex.y * float(texDim.y))), 0);
	outColor = vec4(float(color_data.r) / 255.0f, float(color_data.g) / 255.0f, float(color_data.b) / 255.0f, float(color_data.a) / 255.0f);
})";
	}
}

#endif
