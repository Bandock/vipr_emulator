#ifndef _APPLICATION_HPP_
#define _APPLICATION_HPP_

#include "cosmac_vip.hpp"
#include "renderer.hpp"
#include "gui.hpp"
#include <fmt/core.h>
#include <memory>
#include <map>
#include <SDL.h>

namespace VIPR_Emulator
{
	struct WindowDeleter
	{
		void operator()(SDL_Window *window)
		{
			SDL_DestroyWindow(window);
			fmt::print("Window Successfully Destroyed.\n");
		}
	};

	using Window = std::unique_ptr<SDL_Window, WindowDeleter>;

	struct VersionData
	{
		uint16_t major;
		uint16_t minor;
	};

	enum class OperationMode
	{
		Menu, Input, Machine
	};

	enum class HexKey : uint8_t
	{
		Key_0 = 0x0, Key_1 = 0x1, Key_2 = 0x2, Key_3 = 0x3,
		Key_4 = 0x4, Key_5 = 0x5, Key_6 = 0x6, Key_7 = 0x7,
		Key_8 = 0x8, Key_9 = 0x9, Key_A = 0xA, Key_B = 0xB,
		Key_C = 0xC, Key_D = 0xD, Key_E = 0xE, Key_F = 0xF
	};

	struct ScancodeModData
	{
		SDL_Scancode scancode;
		uint16_t modifiers;
	};

	class Application;

	using KeyCallback = void (*)(Application *, SDL_Scancode, uint16_t);

	class Application
	{
		public:
			Application();
			~Application();
			void RunMainLoop();

			inline bool Fail() const
			{
				return fail;
			}

			void InitializeKeyMaps();

			int GetReturnCode() const;
			void DrawCurrentMenu();

			friend void menu_key_down(Application *app, SDL_Scancode scancode, uint16_t modifiers);
			friend void menu_key_up(Application *app, SDL_Scancode scancode, uint16_t modifiers);

			friend void input_key_down(Application *app, SDL_Scancode scancode, uint16_t modifiers);
			friend void input_key_up(Application *app, SDL_Scancode scancode, uint16_t modifiers);

			friend void machine_key_down(Application *app, SDL_Scancode scancode, uint16_t modifiers);
			friend void machine_key_up(Application *app, SDL_Scancode scancode, uint16_t modifiers);

			friend void main_menu_left(GUI::Menu &obj, void *userdata);
			friend void main_menu_right(GUI::Menu &obj, void *userdata);
			friend void main_menu_activate(GUI::Menu &obj, void *userdata);

			friend void machine_options_left(GUI::Menu &obj, void *userdata);
			friend void machine_options_right(GUI::Menu &obj, void *userdata);
			friend void machine_options_activate(GUI::Menu &obj, void *userdata);
			friend void machine_options_ram_in_kb_input_complete(GUI::Value &obj, void *userdata);
			friend void machine_options_rom_file_input_complete(GUI::Input &obj, void *userdata);

			friend void expansion_board_options_activate(GUI::Menu &obj, void *userdata);

			friend void machine_memory_transfer_left(GUI::Menu &obj, void *userdata);
			friend void machine_memory_transfer_right(GUI::Menu &obj, void *userdata);
			friend void machine_memory_transfer_activate(GUI::Menu &obj, void *userdata);
			friend void machine_memory_transfer_memory_file_input_complete(GUI::Input &obj, void *userdata);
			friend void machine_memory_transfer_start_address_input_complete(GUI::Value &obj, void *userdata);
			friend void machine_memory_transfer_size_input_complete(GUI::Value &obj, void *userdata);

			friend void emulator_options_up(GUI::Menu &obj, void *userdata);
			friend void emulator_options_down(GUI::Menu &obj, void *userdata);
			friend void emulator_options_left(GUI::Menu &obj, void *userdata);
			friend void emulator_options_right(GUI::Menu &obj, void *userdata);
			friend void emulator_options_activate(GUI::Menu &obj, void *userdata);
		private:
			Window MainWindow;
			Renderer MainRenderer;
			KeyCallback key_down_callback, key_up_callback;
			OperationMode current_operation_mode;
			uint8_t current_hex_key;
			std::map<HexKey, SDL_Scancode> Hex_KeyMap;
			std::multimap<char, ScancodeModData> Printable_KeyMap;
			COSMAC_VIP System;
			GUI::Menu MainMenu, MachineOptionsMenu, ExpansionBoardOptionsMenu, MachineMemoryTransferMenu, EmulatorOptionsMenu;
			GUI::Menu *CurrentMenu;
			GUI::ElementData *InputFocus;
			bool exit;
			bool fail;
			int retcode;
			const VersionData version = { 0, 2 };

			void SetOperationMode(OperationMode mode);
			void ConstructMenus();
	};

	consteval uint32_t GetDefaultWindowFlags()
	{
		uint32_t flags = 0x00000000;
		if constexpr (CheckCompatibleRendererType<renderer_type, RendererType::OpenGL_21, RendererType::OpenGL_30, RendererType::OpenGLES_2, RendererType::OpenGLES_3>())
		{
			flags |= SDL_WINDOW_OPENGL;
		}
		return flags;
	}

	void menu_key_down(Application *app, SDL_Scancode scancode, uint16_t modifiers);
	void menu_key_up(Application *app, SDL_Scancode scancode, uint16_t modifiers);

	void input_key_down(Application *app, SDL_Scancode scancode, uint16_t modifiers);
	void input_key_up(Application *app, SDL_Scancode scancode, uint16_t modifiers);

	void machine_key_down(Application *app, SDL_Scancode scancode, uint16_t modifiers);
	void machine_key_up(Application *app, SDL_Scancode scancode, uint16_t modifiers);

	void main_menu_down(GUI::Menu &obj, void *userdata);
	void main_menu_up(GUI::Menu &obj, void *userdata);
	void main_menu_left(GUI::Menu &obj, void *userdata);
	void main_menu_right(GUI::Menu &obj, void *userdata);
	void main_menu_activate(GUI::Menu &obj, void *userdata);

	void machine_options_up(GUI::Menu &obj, void *userdata);
	void machine_options_down(GUI::Menu &obj, void *userdata);
	void machine_options_left(GUI::Menu &obj, void *userdata);
	void machine_options_right(GUI::Menu &obj, void *userdata);
	void machine_options_activate(GUI::Menu &obj, void *userdata);
	void machine_options_ram_in_kb_input_complete(GUI::Value &obj, void *userdata);
	void machine_options_rom_file_input_complete(GUI::Input &obj, void *userdata);

	void expansion_board_options_up(GUI::Menu &obj, void *userdata);
	void expansion_board_options_down(GUI::Menu &obj, void *userdata);
	void expansion_board_options_left(GUI::Menu &obj, void *userdata);
	void expansion_board_options_right(GUI::Menu &obj, void *userdata);
	void expansion_board_options_activate(GUI::Menu &obj, void *userdata);

	void machine_memory_transfer_up(GUI::Menu &obj, void *userdata);
	void machine_memory_transfer_down(GUI::Menu &obj, void *userdata);
	void machine_memory_transfer_left(GUI::Menu &obj, void *userdata);
	void machine_memory_transfer_right(GUI::Menu &obj, void *userdata);
	void machine_memory_transfer_activate(GUI::Menu &obj, void *userdata);
	void machine_memory_transfer_memory_file_input_complete(GUI::Input &obj, void *userdata);
	void machine_memory_transfer_start_address_input_complete(GUI::Value &obj, void *userdata);
	void machine_memory_transfer_size_input_complete(GUI::Value &obj, void *userdata);

	void emulator_options_up(GUI::Menu &obj, void *userdata);
	void emulator_options_down(GUI::Menu &obj, void *userdata);
	void emulator_options_left(GUI::Menu &obj, void *userdata);
	void emulator_options_right(GUI::Menu &obj, void *userdata);
	void emulator_options_activate(GUI::Menu &obj, void *userdata);
}

#endif
