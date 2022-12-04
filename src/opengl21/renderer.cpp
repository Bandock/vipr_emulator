#include "renderer.hpp"
#include "shaders.hpp"
#include <fmt/core.h>
#include <memory>
#include <bit>
#include <fstream>
#include <msbtfont/msbtfont.h>

VIPR_Emulator::Renderer::Renderer() : CurrentWindow(nullptr), MainContext(nullptr), PrimaryVertexShaderId(0), SecondaryFramebufferFragmentShaderId(0), FontFragmentShaderId(0), MachineFragmentShaderId(0), SecondaryFramebufferProgramId(0), FontProgramId(0), MachineProgramId(0), CurrentProgramId(0), VAOId(0), VBOId(0), IBOId(0), SecondaryFramebufferTextureId(0), MenuFontTextureId(0), DisplayTextureId(0), CurrentTextureId(0), SFBOId(0), CurrentFBOId(0), FontColorUniformId(0), FontFlagInvertUniformId(0), CurrentDisplayType(DisplayType::Emulator), font_ctrl { FontColorData { 0.0f, 0.0f, 0.0f, 1.0f }, 0x00 }
{
	vertices = {
		Vertex { { -1.0f, 1.0f }, { 0.0f, 1.0f } },
		Vertex { { 1.0f, 1.0f }, { 1.0f, 1.0f } },
		Vertex { { -1.0f, -1.0f }, { 0.0f, 0.0f } },
		Vertex { { 1.0f, -1.0f }, { 1.0f, 0.0f } }
	};
	indices = { 0, 2, 1, 2, 3, 1 };
}

VIPR_Emulator::Renderer::~Renderer()
{
	if (MainContext != nullptr)
	{
		glUseProgram(0);
		if (CurrentFBOId != 0)
		{
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		}
		if (SFBOId != 0)
		{
			glDeleteFramebuffers(1, &SFBOId);
		}
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glBindVertexArray(0);
		if (DisplayTextureId != 0)
		{
			glDeleteTextures(1, &DisplayTextureId);
		}
		if (MenuFontTextureId != 0)
		{
			glDeleteTextures(1, &MenuFontTextureId);
		}
		if (SecondaryFramebufferTextureId != 0)
		{
			glDeleteTextures(1, &SecondaryFramebufferTextureId);
		}
		if (IBOId != 0)
		{
			glDeleteBuffers(1, &IBOId);
		}
		if (VBOId != 0)
		{
			glDeleteBuffers(1, &VBOId);
		}
		if (VAOId != 0)
		{
			glDeleteVertexArrays(1, &VAOId);
		}
		if (MachineProgramId != 0)
		{
			glDetachShader(MachineProgramId, PrimaryVertexShaderId);
			glDetachShader(MachineProgramId, MachineFragmentShaderId);
			glDeleteProgram(MachineProgramId);
		}
		if (FontProgramId != 0)
		{
			glDetachShader(FontProgramId, PrimaryVertexShaderId);
			glDetachShader(FontProgramId, FontFragmentShaderId);
			glDeleteProgram(FontProgramId);
		}
		if (SecondaryFramebufferProgramId != 0)
		{
			glDetachShader(SecondaryFramebufferProgramId, PrimaryVertexShaderId);
			glDetachShader(SecondaryFramebufferProgramId, SecondaryFramebufferFragmentShaderId);
			glDeleteProgram(SecondaryFramebufferProgramId);
		}
		if (MachineFragmentShaderId != 0)
		{
			glDeleteShader(MachineFragmentShaderId);
		}
		if (FontFragmentShaderId != 0)
		{
			glDeleteShader(FontFragmentShaderId);
		}
		if (SecondaryFramebufferFragmentShaderId != 0)
		{
			glDeleteShader(SecondaryFramebufferFragmentShaderId);
		}
		if (PrimaryVertexShaderId != 0)
		{
			glDeleteShader(PrimaryVertexShaderId);
		}
		SDL_GL_DeleteContext(MainContext);
	}
}

bool VIPR_Emulator::Renderer::Setup(SDL_Window *window)
{
	if (MainContext == nullptr && window != nullptr)
	{
		CurrentWindow = window;
		MainContext = SDL_GL_CreateContext(CurrentWindow);
		if (MainContext == nullptr)
		{
			fmt::print("{}\n", SDL_GetError());
			return false;
		}
		GLenum err = glewInit();
		if (err != GLEW_OK)
		{
			fmt::print("GLEW Initialization failed.\n");
			return false;
		}
		if (!GLEW_ARB_framebuffer_object)
		{
			fmt::print("Framebuffer objects are not supported on this platform.\n");
			return false;
		}
		if (!GLEW_ARB_texture_storage)
		{
			fmt::print("Texture storage functions are not supported on this platform.\n");
			return false;
		}
		if (!CompileShader(PrimaryVertexShaderId, GL_VERTEX_SHADER, Shader::PrimaryVertexShader))
		{
			return false;
		}
		if (!CompileShader(SecondaryFramebufferFragmentShaderId, GL_FRAGMENT_SHADER, Shader::SecondaryFramebufferFragmentShader))
		{
			return false;
		}
		if (!CompileShader(FontFragmentShaderId, GL_FRAGMENT_SHADER, Shader::FontFragmentShader))
		{
			return false;
		}
		if (!CompileShader(MachineFragmentShaderId, GL_FRAGMENT_SHADER, Shader::MachineFragmentShader))
		{
			return false;
		}
		std::vector<GLuint> shader_list;
		shader_list.push_back(PrimaryVertexShaderId);
		shader_list.push_back(SecondaryFramebufferFragmentShaderId);
		if (!LinkProgram(SecondaryFramebufferProgramId, std::move(shader_list)))
		{
			return false;
		}
		shader_list.push_back(PrimaryVertexShaderId);
		shader_list.push_back(FontFragmentShaderId);
		if (!LinkProgram(FontProgramId, std::move(shader_list)))
		{
			return false;
		}
		shader_list.push_back(PrimaryVertexShaderId);
		shader_list.push_back(MachineFragmentShaderId);
		if (!LinkProgram(MachineProgramId, std::move(shader_list)))
		{
			return false;
		}
		FontColorUniformId = glGetUniformLocation(FontProgramId, "FontColor");
		FontFlagInvertUniformId = glGetUniformLocation(FontProgramId, "FontFlag_Invert");
		glGenVertexArrays(1, &VAOId);
		glGenBuffers(1, &VBOId);
		glGenBuffers(1, &IBOId);
		glBindBuffer(GL_ARRAY_BUFFER, VBOId);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBOId);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint8_t), indices.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void *>(0));
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void *>(sizeof(float) * 2));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glGenTextures(1, &SecondaryFramebufferTextureId);
		glGenTextures(1, &MenuFontTextureId);
		glGenTextures(1, &DisplayTextureId);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, SecondaryFramebufferTextureId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 640, 320);
		glBindTexture(GL_TEXTURE_2D, MenuFontTextureId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 128, 48);
		{
			std::ifstream vipr_font_file("vipr_menu_font.mbft", std::ios::binary);
			if (vipr_font_file.fail())
			{
				fmt::print("Unable to load 'vipr_menu_font.mbft'.\n");
				return false;
			}
			msbtfont_header header;
			msbtfont_filedata filedata;
			vipr_font_file.read(reinterpret_cast<char *>(&header), sizeof(header));
			msbtfont_create_filedata(&header, &filedata);
			vipr_font_file.read(reinterpret_cast<char *>(filedata.data), filedata.size);
			msbtfont_surface_descriptor surface_desc;
			surface_desc.rect.x = 0;
			surface_desc.rect.y = 0;
			surface_desc.rect.width = 128;
			surface_desc.rect.height = 48;
			surface_desc.format = MSBTFONT_SURFACE_FORMAT_8;
			surface_desc.origin = MSBTFONT_SURFACE_ORIGIN_LOWERLEFT;
			size_t surface_memory_req = msbtfont_get_surface_memory_requirement(&surface_desc);
			std::vector<uint8_t> font_surface(surface_memory_req);
			msbtfont_copy_to_surface(&header, &filedata, 16, 0, &surface_desc, font_surface.data());
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 128, 48, GL_RED, GL_UNSIGNED_BYTE, font_surface.data());
			msbtfont_delete_filedata(&filedata);
		}
		glBindTexture(GL_TEXTURE_2D, DisplayTextureId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 64, 128);
		glBindTexture(GL_TEXTURE_2D, SecondaryFramebufferTextureId);
		CurrentTextureId = SecondaryFramebufferTextureId;
		glGenFramebuffers(1, &SFBOId);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, SFBOId);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, SecondaryFramebufferTextureId, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glViewport(0, 0, 1280, 640);
		glUseProgram(SecondaryFramebufferProgramId);
		CurrentProgramId = SecondaryFramebufferProgramId;
		SDL_GL_SetSwapInterval(0);
		return true;
	}	
	return false;
}

void VIPR_Emulator::Renderer::Render()
{
	if (CurrentFBOId != 0)
	{
		CurrentFBOId = 0;
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}
	switch (CurrentDisplayType)
	{
		case DisplayType::Emulator:
		{
			if (CurrentProgramId != SecondaryFramebufferProgramId)
			{
				CurrentProgramId = SecondaryFramebufferProgramId;
				glUseProgram(SecondaryFramebufferProgramId);
			}
			if (CurrentTextureId != SecondaryFramebufferTextureId)
			{
				CurrentTextureId = SecondaryFramebufferTextureId;
				glBindTexture(GL_TEXTURE_2D, SecondaryFramebufferTextureId);
			}
			break;
		}
		case DisplayType::Machine:
		{
			if (CurrentProgramId != MachineProgramId)
			{
				CurrentProgramId = MachineProgramId;
				glUseProgram(MachineProgramId);
			}
			if (CurrentTextureId != DisplayTextureId)
			{
				CurrentTextureId = DisplayTextureId;
				glBindTexture(GL_TEXTURE_2D, DisplayTextureId);
			}
			break;
		}
	}
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_BYTE, reinterpret_cast<void *>(0));
	SDL_GL_SwapWindow(CurrentWindow);
}

void VIPR_Emulator::Renderer::ClearSecondaryFramebuffer()
{
	if (CurrentFBOId != SFBOId)
	{
		CurrentFBOId = SFBOId;
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, SFBOId);
	}
	glClear(GL_COLOR_BUFFER_BIT);
}

void VIPR_Emulator::Renderer::ClearDisplay()
{
	if (CurrentTextureId != DisplayTextureId)
	{
		CurrentTextureId = DisplayTextureId;
		glBindTexture(GL_TEXTURE_2D, DisplayTextureId);
	}
	std::array<uint8_t, 64 * 128> buffer;
	memset(buffer.data(), 0, buffer.size());
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 64, 128, GL_RED, GL_UNSIGNED_BYTE, buffer.data());
}

void VIPR_Emulator::Renderer::SetDisplayType(DisplayType type)
{
	CurrentDisplayType = type;
}

VIPR_Emulator::DisplayType VIPR_Emulator::Renderer::GetDisplayType() const
{
	return CurrentDisplayType;
}

void VIPR_Emulator::Renderer::SetFontColor(uint8_t r, uint8_t g, uint8_t b)
{
	FontColorData color = { r / 255.0f, g / 255.0f, b / 255.0f, 1.0f };
	if (font_ctrl.FontColor.r != color.r || font_ctrl.FontColor.g != color.g || font_ctrl.FontColor.b != color.b)
	{
		font_ctrl.FontColor = color;
	}
}

void VIPR_Emulator::Renderer::SetFontFlags(uint32_t flags)
{
	if (font_ctrl.FontFlags != flags)
	{
		font_ctrl.FontFlags = flags;
	}
}

void VIPR_Emulator::Renderer::DrawChar(char character, uint16_t x, uint16_t y)
{
	if (CurrentTextureId != MenuFontTextureId)
	{
		CurrentTextureId = MenuFontTextureId;
		glBindTexture(GL_TEXTURE_2D, MenuFontTextureId);
	}
	if (CurrentFBOId != SFBOId)
	{
		CurrentFBOId = SFBOId;
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, SFBOId);
	}
	if (CurrentProgramId != FontProgramId)
	{
		CurrentProgramId = FontProgramId;
		glUseProgram(FontProgramId);
	}
	glUniform4fv(FontColorUniformId, 1, reinterpret_cast<const float *>(&font_ctrl.FontColor));
	glUniform1i(FontFlagInvertUniformId, font_ctrl.FontFlags & 0x01);
	glViewport(0, 0, 640, 320);
	std::array<Vertex, 4> original_vertices = vertices;
	float left_x = (x / 320.0f) - 1.0f;
	float right_x = ((x + 8) / 320.0f) - 1.0f;
	float up_y = 1.0f - (y / 160.0f);
	float down_y = 1.0f - ((y + 8) / 160.0f);
	char current_character = character - 32;
	float tex_left_x = (current_character % 16 * 8) / 128.0f;
	float tex_right_x = ((current_character % 16 * 8) + 8) / 128.0f;
	float tex_up_y = 1.0f - ((current_character / 16 * 8) / 48.0f);
	float tex_down_y = 1.0f - (((current_character / 16 * 8) + 8) / 48.0f);
	vertices = {
		Vertex { { left_x, up_y }, { tex_left_x, tex_up_y } },
		Vertex { { right_x, up_y }, { tex_right_x, tex_up_y } },
		Vertex { { left_x, down_y }, { tex_left_x, tex_down_y } },
		Vertex { { right_x, down_y }, { tex_right_x, tex_down_y } }
	};
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data());
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_BYTE, reinterpret_cast<void *>(0));
	vertices = original_vertices;
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data());
	glViewport(0, 0, 1280, 640);
}

void VIPR_Emulator::Renderer::DrawText(std::string text, uint16_t x, uint16_t y)
{
	if (CurrentTextureId != MenuFontTextureId)
	{
		CurrentTextureId = MenuFontTextureId;
		glBindTexture(GL_TEXTURE_2D, MenuFontTextureId);
	}
	if (CurrentFBOId != SFBOId)
	{
		CurrentFBOId = SFBOId;
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, SFBOId);
	}
	if (CurrentProgramId != FontProgramId)
	{
		CurrentProgramId = FontProgramId;
		glUseProgram(FontProgramId);
	}
	glUniform4fv(FontColorUniformId, 1, reinterpret_cast<const float *>(&font_ctrl.FontColor));
	glUniform1i(FontFlagInvertUniformId, font_ctrl.FontFlags & 0x01);
	glViewport(0, 0, 640, 320);
	std::array<Vertex, 4> original_vertices = vertices;
	float up_y = 1.0f - (y / 160.0f);
	float down_y = 1.0f - ((y + 8) / 160.0f);
	for (size_t i = 0; i < text.size(); ++i)
	{
		if (text[i] < 32 || text[i] > 126)
		{
			continue;
		}
		float left_x = ((x + (i * 8)) / 320.0f) - 1.0f;
		float right_x = ((x + (i * 8) + 8) / 320.0f) - 1.0f;
		char current_character = text[i] - 32;
		float tex_left_x = (current_character % 16 * 8) / 128.0f;
		float tex_right_x = ((current_character % 16 * 8) + 8) / 128.0f;
		float tex_up_y = 1.0f - ((current_character / 16 * 8) / 48.0f);
		float tex_down_y = 1.0f - (((current_character / 16 * 8) + 8) / 48.0f);
		vertices = {
			Vertex { { left_x, up_y }, { tex_left_x, tex_up_y } },
			Vertex { { right_x, up_y }, { tex_right_x, tex_up_y } },
			Vertex { { left_x, down_y }, { tex_left_x, tex_down_y } },
			Vertex { { right_x, down_y }, { tex_right_x, tex_down_y } }
		};
		glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data());
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_BYTE, reinterpret_cast<void *>(0));
	}
	vertices = original_vertices;
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data());
	glViewport(0, 0, 1280, 640);
}

void VIPR_Emulator::Renderer::DrawByte(uint8_t data, uint8_t line, uint8_t offset)
{
	if (CurrentTextureId != DisplayTextureId)
	{
		CurrentTextureId = DisplayTextureId;
		glBindTexture(GL_TEXTURE_2D, DisplayTextureId);
	}
	std::array<uint8_t, 8> buffer;
	uint8_t flag = 0x80;
	uint8_t b_offset = 0;
	for (uint8_t i = 0; i < buffer.size(); ++i)
	{
		buffer[i] = ((data & flag) >> (7 - b_offset));
		flag >>= 1;
		++b_offset;
	}
	memcpy(&display_buffer[((127 - line) * 64) + (offset * 8)], buffer.data(), buffer.size());
	if (line == 127 && offset == 7)
	{
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 64, 128, GL_RED, GL_UNSIGNED_BYTE, display_buffer.data());
	}
}

bool VIPR_Emulator::Renderer::CompileShader(GLuint &shader, GLuint shader_type, const char *shader_code)
{
	if (shader != 0)
	{
		glDeleteShader(shader);
	}
	shader = glCreateShader(shader_type);
	glShaderSource(shader, 1, &shader_code, nullptr);
	glCompileShader(shader);
	int compile_status = 0, info_log_len = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
	if (compile_status == GL_FALSE)
	{
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_len);
		std::vector<char> info_log(info_log_len);
		glGetShaderInfoLog(shader, info_log_len, nullptr, info_log.data());
		fmt::print("{}\n", info_log.data());
		return false;
	}
	return true;
}

bool VIPR_Emulator::Renderer::LinkProgram(GLuint &program, std::vector<GLuint> shader_list)
{
	if (shader_list.size() == 0)
	{
		return false;
	}
	if (program != 0)
	{
		glDeleteProgram(program);
	}
	program = glCreateProgram();
	for (auto &i : shader_list)
	{
		glAttachShader(program, i);
	}
	glLinkProgram(program);
	int link_status = 0, info_log_len = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &link_status);
	if (link_status == GL_FALSE)
	{
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_len);
		std::vector<char> info_log(info_log_len);
		glGetProgramInfoLog(program, info_log_len, nullptr, info_log.data());
		fmt::print("{}\n", info_log.data());
		return false;
	}
	return true;
}
