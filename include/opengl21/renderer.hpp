#ifndef _RENDERER_HPP_
#define _RENDERER_HPP_

#include <cstdint>
#include <SDL.h>
#include <GL/glew.h>
#include <string>
#include <array>
#include <vector>
#include "renderer_type.hpp"

namespace VIPR_Emulator
{
	enum class DisplayType
	{
		Emulator, Machine
	};

	struct Vertex
	{
		std::array<float, 2> pos;
		std::array<float, 2> tex;
	};

	struct FontColorData
	{
		float r;
		float g;
		float b;
		float a;
	};

	struct FontControlData
	{
		FontColorData FontColor;
		uint32_t FontFlags;
	};

	const RendererType renderer_type = RendererType::OpenGL_21;

	class Renderer
	{
		public:
			Renderer();
			~Renderer();
			bool Setup(SDL_Window *window);
			void Render();
			void ClearSecondaryFramebuffer();
			void ClearDisplay();
			void SetDisplayType(DisplayType type);
			DisplayType GetDisplayType() const;
			void SetFontColor(uint8_t r, uint8_t g, uint8_t b);
			void SetFontFlags(uint32_t flags);
			void DrawChar(char character, uint16_t x, uint16_t y);
			void DrawText(std::string text, uint16_t x, uint16_t y);
			void DrawByte(uint8_t data, uint8_t line, uint8_t offset);
		private:
			SDL_Window *CurrentWindow;
			SDL_GLContext MainContext;
			GLuint PrimaryVertexShaderId, SecondaryFramebufferFragmentShaderId,
			       FontFragmentShaderId, MachineFragmentShaderId, SecondaryFramebufferProgramId,
			       FontProgramId, MachineProgramId, CurrentProgramId, VAOId, VBOId, IBOId,
			       SecondaryFramebufferTextureId, MenuFontTextureId, DisplayTextureId,
			       CurrentTextureId, SFBOId, CurrentFBOId;
			GLint FontColorUniformId, FontFlagInvertUniformId;
			DisplayType CurrentDisplayType;
			FontControlData font_ctrl;
			std::array<Vertex, 4> vertices;
			std::array<uint8_t, 6> indices;
			std::array<uint8_t, 64 * 128> display_buffer;

			bool CompileShader(GLuint &shader, GLuint shader_type, const char *shader_code);
			bool LinkProgram(GLuint &program, std::vector<GLuint> shader_list);
	};
}

#endif
