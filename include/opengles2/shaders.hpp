#ifndef _SHADERS_HPP_
#define _SHADERS_HPP_

namespace VIPR_Emulator
{
	namespace Shader
	{
		const char *PrimaryVertexShader = R"(#version 100

attribute vec2 pos;
attribute vec2 tex;

varying vec2 outTex;

void main()
{
	gl_Position = vec4(pos, 0.0, 1.0);
	outTex = tex;
})";
		const char *SecondaryFramebufferFragmentShader = R"(#version 100

precision highp float;
precision highp sampler2D;

varying vec2 outTex;

uniform sampler2D SecondaryFramebufferTexture;

void main()
{
	gl_FragColor = texture2D(SecondaryFramebufferTexture, outTex);
})";
		const char *FontFragmentShader = R"(#version 100

precision highp float;
precision highp int;
precision highp sampler2D;

uniform vec4 FontColor;
uniform bool FontFlag_Invert;

varying vec2 outTex;

uniform sampler2D CurrentTexture;

void main()
{
	vec4 color_data = texture2D(CurrentTexture, outTex);
	gl_FragColor = (int(color_data.r * 256.0) == (FontFlag_Invert ? 0 : 1)) ? FontColor : vec4(0.0, 0.0, 0.0, 1.0);
})";
		const char *MachineFragmentShader = R"(#version 100

precision highp float;
precision highp int;
precision highp sampler2D;

varying vec2 outTex;

uniform sampler2D DisplayTexture;

void main()
{
	vec4 color_data = texture2D(DisplayTexture, outTex);
	gl_FragColor = color_data;
})";

	}
}

#endif
