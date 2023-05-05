#include "application.hpp"
#include <chrono>
#include <fstream>
#include <sstream>
#include <ranges>

VIPR_Emulator::Application::Application() : current_hex_key(0x0), key_down_callback(VIPR_Emulator::machine_key_down), key_up_callback(VIPR_Emulator::machine_key_up), current_operation_mode(OperationMode::Menu), InputFocus(nullptr), exit(false), fail(false), retcode(0)
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	if (System.Fail())
	{
		fail = true;
		retcode = -1;
		return;
	}
	if constexpr (CheckCompatibleRendererType<renderer_type, RendererType::OpenGL_21, RendererType::OpenGL_30, RendererType::OpenGLES_2, RendererType::OpenGLES_3>())
	{
		if constexpr (CheckCompatibleRendererType<renderer_type, RendererType::OpenGL_21>())
		{
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
		}
		else if constexpr (CheckCompatibleRendererType<renderer_type, RendererType::OpenGL_30>())
		{
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
		}
		else if constexpr (CheckCompatibleRendererType<renderer_type, RendererType::OpenGLES_2>())
		{
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
		}
		else if constexpr (CheckCompatibleRendererType<renderer_type, RendererType::OpenGLES_3>())
		{
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
		}
		if constexpr (CheckCompatibleRendererType<renderer_type, RendererType::OpenGLES_2, RendererType::OpenGLES_3>())
		{
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
		}
	}
	MainWindow = Window(SDL_CreateWindow("VIPR Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 640, GetDefaultWindowFlags()));
	if (!MainRenderer.Setup(MainWindow.get()))
	{
		fail = true;
		retcode = -1;
		return;
	}
	System.SetupDisplay(&MainRenderer);
	InitializeKeyMaps();
	ConstructMenus();
	GUI::MultiChoice *OutputAudioDevice = std::get_if<GUI::MultiChoice>(&EmulatorOptionsMenu.element_list[1].element);
	System.SetupAudio(OutputAudioDevice->choice_list[OutputAudioDevice->current_choice]);
	CurrentMenu = &MainMenu;
	DrawCurrentMenu();
	SetOperationMode(OperationMode::Menu);
}

VIPR_Emulator::Application::~Application()
{
	SDL_Quit();
}

void VIPR_Emulator::Application::RunMainLoop()
{
	while (!exit)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_KEYDOWN:
				{
					if (key_down_callback != nullptr)
					{
						key_down_callback(this, event.key.keysym.scancode, event.key.keysym.mod);
					}
					break;
				}
				case SDL_KEYUP:
				{
					if (key_up_callback != nullptr)
					{
						key_up_callback(this, event.key.keysym.scancode, event.key.keysym.mod);
					}
					break;
				}
				case SDL_AUDIODEVICEADDED:
				{
					std::vector<std::string> OutputAudioDeviceList(SDL_GetNumAudioDevices(0));
					for (size_t i = 0; i < OutputAudioDeviceList.size(); ++i)
					{
						OutputAudioDeviceList[i] = SDL_GetAudioDeviceName(i, 0);
					}
					GUI::MultiChoice *OutputAudioDevice = std::get_if<GUI::MultiChoice>(&EmulatorOptionsMenu.element_list[1].element);
					OutputAudioDevice->choice_list = std::move(OutputAudioDeviceList);
					size_t choice_count = OutputAudioDevice->choice_list.size();
					if (OutputAudioDevice->current_choice >= choice_count)
					{
						if (choice_count > 0)
						{
							OutputAudioDevice->current_choice = OutputAudioDevice->choice_list.size() - 1;
							System.SetupAudio(OutputAudioDevice->choice_list[OutputAudioDevice->current_choice]);
						}
					}
					if (CurrentMenu == &EmulatorOptionsMenu)
					{
						DrawCurrentMenu();
					}
					break;
				}
				case SDL_AUDIODEVICEREMOVED:
				{
					break;
				}
				case SDL_QUIT:
				{
					exit = true;
					break;
				}
			}
		}
		if (System.IsRunning() && current_operation_mode == OperationMode::Machine)
		{
			System.RunMachine(std::chrono::high_resolution_clock::now());
		}
		else
		{
			MainRenderer.Render();
		}
	}
}

void VIPR_Emulator::Application::InitializeKeyMaps()
{
	Hex_KeyMap[HexKey::Key_1] = SDL_SCANCODE_1;
	Hex_KeyMap[HexKey::Key_2] = SDL_SCANCODE_2;
	Hex_KeyMap[HexKey::Key_3] = SDL_SCANCODE_3;
	Hex_KeyMap[HexKey::Key_C] = SDL_SCANCODE_4;
	Hex_KeyMap[HexKey::Key_4] = SDL_SCANCODE_Q;
	Hex_KeyMap[HexKey::Key_5] = SDL_SCANCODE_W;
	Hex_KeyMap[HexKey::Key_6] = SDL_SCANCODE_E;
	Hex_KeyMap[HexKey::Key_D] = SDL_SCANCODE_R;
	Hex_KeyMap[HexKey::Key_7] = SDL_SCANCODE_A;
	Hex_KeyMap[HexKey::Key_8] = SDL_SCANCODE_S;
	Hex_KeyMap[HexKey::Key_9] = SDL_SCANCODE_D;
	Hex_KeyMap[HexKey::Key_E] = SDL_SCANCODE_F;
	Hex_KeyMap[HexKey::Key_A] = SDL_SCANCODE_Z;
	Hex_KeyMap[HexKey::Key_0] = SDL_SCANCODE_X;
	Hex_KeyMap[HexKey::Key_B] = SDL_SCANCODE_C;
	Hex_KeyMap[HexKey::Key_F] = SDL_SCANCODE_V;

	Hex_KeyMap_2[HexKey::Key_1] = SDL_SCANCODE_7;
	Hex_KeyMap_2[HexKey::Key_2] = SDL_SCANCODE_8;
	Hex_KeyMap_2[HexKey::Key_3] = SDL_SCANCODE_9;
	Hex_KeyMap_2[HexKey::Key_C] = SDL_SCANCODE_0;
	Hex_KeyMap_2[HexKey::Key_4] = SDL_SCANCODE_U;
	Hex_KeyMap_2[HexKey::Key_5] = SDL_SCANCODE_I;
	Hex_KeyMap_2[HexKey::Key_6] = SDL_SCANCODE_O;
	Hex_KeyMap_2[HexKey::Key_D] = SDL_SCANCODE_P;
	Hex_KeyMap_2[HexKey::Key_7] = SDL_SCANCODE_J;
	Hex_KeyMap_2[HexKey::Key_8] = SDL_SCANCODE_K;
	Hex_KeyMap_2[HexKey::Key_9] = SDL_SCANCODE_L;
	Hex_KeyMap_2[HexKey::Key_E] = SDL_SCANCODE_SEMICOLON;
	Hex_KeyMap_2[HexKey::Key_A] = SDL_SCANCODE_N;
	Hex_KeyMap_2[HexKey::Key_0] = SDL_SCANCODE_M;
	Hex_KeyMap_2[HexKey::Key_B] = SDL_SCANCODE_COMMA;
	Hex_KeyMap_2[HexKey::Key_F] = SDL_SCANCODE_PERIOD;

	auto InsertNonAlphaCharacter = [this](const char character, const SDL_Scancode scancode)
	{
		Printable_KeyMap.insert(std::pair<char, ScancodeModData>(character, { scancode, 0 }));
		Printable_KeyMap.insert(std::pair<char, ScancodeModData>(character, { scancode, KMOD_CAPS }));
		Printable_KeyMap.insert(std::pair<char, ScancodeModData>(character, { scancode, KMOD_NUM } ));
		Printable_KeyMap.insert(std::pair<char, ScancodeModData>(character, { scancode, KMOD_NUM | KMOD_CAPS }));
	};

	auto InsertShiftCharacter = [this](const char character, const SDL_Scancode scancode)
	{
		Printable_KeyMap.insert(std::pair<char, ScancodeModData>(character, { scancode, KMOD_LSHIFT }));
		Printable_KeyMap.insert(std::pair<char, ScancodeModData>(character, { scancode, KMOD_RSHIFT }));
		Printable_KeyMap.insert(std::pair<char, ScancodeModData>(character, { scancode, KMOD_SHIFT }));
		Printable_KeyMap.insert(std::pair<char, ScancodeModData>(character, { scancode, KMOD_CAPS | KMOD_LSHIFT }));
		Printable_KeyMap.insert(std::pair<char, ScancodeModData>(character, { scancode, KMOD_CAPS | KMOD_RSHIFT }));
		Printable_KeyMap.insert(std::pair<char, ScancodeModData>(character, { scancode, KMOD_CAPS | KMOD_SHIFT }));
		Printable_KeyMap.insert(std::pair<char, ScancodeModData>(character, { scancode, KMOD_NUM | KMOD_LSHIFT }));
		Printable_KeyMap.insert(std::pair<char, ScancodeModData>(character, { scancode, KMOD_NUM | KMOD_RSHIFT }));
		Printable_KeyMap.insert(std::pair<char, ScancodeModData>(character, { scancode, KMOD_NUM | KMOD_SHIFT }));
		Printable_KeyMap.insert(std::pair<char, ScancodeModData>(character, { scancode, KMOD_NUM | KMOD_CAPS | KMOD_LSHIFT }));
		Printable_KeyMap.insert(std::pair<char, ScancodeModData>(character, { scancode, KMOD_NUM | KMOD_CAPS | KMOD_RSHIFT }));
		Printable_KeyMap.insert(std::pair<char, ScancodeModData>(character, { scancode, KMOD_NUM | KMOD_CAPS | KMOD_SHIFT }));
	};

	InsertNonAlphaCharacter(' ', SDL_SCANCODE_SPACE);
	InsertShiftCharacter(' ', SDL_SCANCODE_SPACE);
	InsertShiftCharacter('!', SDL_SCANCODE_1);
	InsertShiftCharacter('"', SDL_SCANCODE_APOSTROPHE);
	InsertShiftCharacter('#', SDL_SCANCODE_3);
	InsertShiftCharacter('$', SDL_SCANCODE_4);
	InsertShiftCharacter('%', SDL_SCANCODE_5);
	InsertShiftCharacter('&', SDL_SCANCODE_7);
	InsertNonAlphaCharacter('\'', SDL_SCANCODE_APOSTROPHE);
	InsertShiftCharacter('(', SDL_SCANCODE_9);
	InsertShiftCharacter(')', SDL_SCANCODE_0);
	InsertShiftCharacter('*', SDL_SCANCODE_8);
	InsertShiftCharacter('+', SDL_SCANCODE_EQUALS);
	InsertNonAlphaCharacter(',', SDL_SCANCODE_COMMA);
	InsertNonAlphaCharacter('-', SDL_SCANCODE_MINUS);
	InsertNonAlphaCharacter('.', SDL_SCANCODE_PERIOD);
	InsertNonAlphaCharacter('/', SDL_SCANCODE_SLASH);

	InsertNonAlphaCharacter('0', SDL_SCANCODE_0);
	InsertNonAlphaCharacter('1', SDL_SCANCODE_1);
	InsertNonAlphaCharacter('2', SDL_SCANCODE_2);
	InsertNonAlphaCharacter('3', SDL_SCANCODE_3);
	InsertNonAlphaCharacter('4', SDL_SCANCODE_4);
	InsertNonAlphaCharacter('5', SDL_SCANCODE_5);
	InsertNonAlphaCharacter('6', SDL_SCANCODE_6);
	InsertNonAlphaCharacter('7', SDL_SCANCODE_7);
	InsertNonAlphaCharacter('8', SDL_SCANCODE_8);
	InsertNonAlphaCharacter('9', SDL_SCANCODE_9);

	InsertShiftCharacter(':', SDL_SCANCODE_SEMICOLON);
	InsertNonAlphaCharacter(';', SDL_SCANCODE_SEMICOLON);
	InsertShiftCharacter('<',  SDL_SCANCODE_COMMA);
	InsertNonAlphaCharacter('=', SDL_SCANCODE_EQUALS);
	InsertShiftCharacter('>', SDL_SCANCODE_PERIOD);
	InsertShiftCharacter('?', SDL_SCANCODE_SLASH);
	InsertShiftCharacter('@', SDL_SCANCODE_2);

	auto InsertUppercaseCharacter = [this](const char character, const SDL_Scancode scancode)
	{
		if (isupper(character))
		{
			Printable_KeyMap.insert(std::pair<char, ScancodeModData>(character, { scancode, KMOD_LSHIFT }));
			Printable_KeyMap.insert(std::pair<char, ScancodeModData>(character, { scancode, KMOD_RSHIFT }));
			Printable_KeyMap.insert(std::pair<char, ScancodeModData>(character, { scancode, KMOD_SHIFT }));
			Printable_KeyMap.insert(std::pair<char, ScancodeModData>(character, { scancode, KMOD_CAPS }));
			Printable_KeyMap.insert(std::pair<char, ScancodeModData>(character, { scancode, KMOD_NUM | KMOD_LSHIFT }));
			Printable_KeyMap.insert(std::pair<char, ScancodeModData>(character, { scancode, KMOD_NUM | KMOD_RSHIFT }));
			Printable_KeyMap.insert(std::pair<char, ScancodeModData>(character, { scancode, KMOD_NUM | KMOD_SHIFT }));
			Printable_KeyMap.insert(std::pair<char, ScancodeModData>(character, { scancode, KMOD_NUM | KMOD_CAPS }));
		}
	};

	auto InsertLowercaseCharacter = [this](const char character, const SDL_Scancode scancode)
	{
		if (islower(character))
		{
			Printable_KeyMap.insert(std::pair<char, ScancodeModData>(character, { scancode, 0 }));
			Printable_KeyMap.insert(std::pair<char, ScancodeModData>(character, { scancode, KMOD_CAPS | KMOD_LSHIFT }));
			Printable_KeyMap.insert(std::pair<char, ScancodeModData>(character, { scancode, KMOD_CAPS | KMOD_RSHIFT }));
			Printable_KeyMap.insert(std::pair<char, ScancodeModData>(character, { scancode, KMOD_CAPS | KMOD_SHIFT }));
			Printable_KeyMap.insert(std::pair<char, ScancodeModData>(character, { scancode, KMOD_NUM}));
			Printable_KeyMap.insert(std::pair<char, ScancodeModData>(character, { scancode, KMOD_NUM | KMOD_CAPS | KMOD_LSHIFT }));
			Printable_KeyMap.insert(std::pair<char, ScancodeModData>(character, { scancode, KMOD_NUM | KMOD_CAPS | KMOD_RSHIFT }));
			Printable_KeyMap.insert(std::pair<char, ScancodeModData>(character, { scancode, KMOD_NUM | KMOD_CAPS | KMOD_SHIFT }));
		}
	};
	
	InsertUppercaseCharacter('A', SDL_SCANCODE_A);
	InsertUppercaseCharacter('B', SDL_SCANCODE_B);
	InsertUppercaseCharacter('C', SDL_SCANCODE_C);
	InsertUppercaseCharacter('D', SDL_SCANCODE_D);
	InsertUppercaseCharacter('E', SDL_SCANCODE_E);
	InsertUppercaseCharacter('F', SDL_SCANCODE_F);
	InsertUppercaseCharacter('G', SDL_SCANCODE_G);
	InsertUppercaseCharacter('H', SDL_SCANCODE_H);
	InsertUppercaseCharacter('I', SDL_SCANCODE_I);
	InsertUppercaseCharacter('J', SDL_SCANCODE_J);
	InsertUppercaseCharacter('K', SDL_SCANCODE_K);
	InsertUppercaseCharacter('L', SDL_SCANCODE_L);
	InsertUppercaseCharacter('M', SDL_SCANCODE_M);
	InsertUppercaseCharacter('N', SDL_SCANCODE_N);
	InsertUppercaseCharacter('O', SDL_SCANCODE_O);
	InsertUppercaseCharacter('P', SDL_SCANCODE_P);
	InsertUppercaseCharacter('Q', SDL_SCANCODE_Q);
	InsertUppercaseCharacter('R', SDL_SCANCODE_R);
	InsertUppercaseCharacter('S', SDL_SCANCODE_S);
	InsertUppercaseCharacter('T', SDL_SCANCODE_T);
	InsertUppercaseCharacter('U', SDL_SCANCODE_U);
	InsertUppercaseCharacter('V', SDL_SCANCODE_V);
	InsertUppercaseCharacter('W', SDL_SCANCODE_W);
	InsertUppercaseCharacter('X', SDL_SCANCODE_X);
	InsertUppercaseCharacter('Y', SDL_SCANCODE_Y);
	InsertUppercaseCharacter('Z', SDL_SCANCODE_Z);

	InsertNonAlphaCharacter('[', SDL_SCANCODE_LEFTBRACKET);
	InsertNonAlphaCharacter('\\', SDL_SCANCODE_BACKSLASH);
	InsertNonAlphaCharacter(']', SDL_SCANCODE_RIGHTBRACKET);
	InsertShiftCharacter('^', SDL_SCANCODE_6);
	InsertShiftCharacter('_', SDL_SCANCODE_MINUS);
	InsertNonAlphaCharacter('`', SDL_SCANCODE_GRAVE);

	InsertLowercaseCharacter('a', SDL_SCANCODE_A);
	InsertLowercaseCharacter('b', SDL_SCANCODE_B);
	InsertLowercaseCharacter('c', SDL_SCANCODE_C);
	InsertLowercaseCharacter('d', SDL_SCANCODE_D);
	InsertLowercaseCharacter('e', SDL_SCANCODE_E);
	InsertLowercaseCharacter('f', SDL_SCANCODE_F);
	InsertLowercaseCharacter('g', SDL_SCANCODE_G);
	InsertLowercaseCharacter('h', SDL_SCANCODE_H);
	InsertLowercaseCharacter('i', SDL_SCANCODE_I);
	InsertLowercaseCharacter('j', SDL_SCANCODE_J);
	InsertLowercaseCharacter('k', SDL_SCANCODE_K);
	InsertLowercaseCharacter('l', SDL_SCANCODE_L);
	InsertLowercaseCharacter('m', SDL_SCANCODE_M);
	InsertLowercaseCharacter('n', SDL_SCANCODE_N);
	InsertLowercaseCharacter('o', SDL_SCANCODE_O);
	InsertLowercaseCharacter('p', SDL_SCANCODE_P);
	InsertLowercaseCharacter('q', SDL_SCANCODE_Q);
	InsertLowercaseCharacter('r', SDL_SCANCODE_R);
	InsertLowercaseCharacter('s', SDL_SCANCODE_S);
	InsertLowercaseCharacter('t', SDL_SCANCODE_T);
	InsertLowercaseCharacter('u', SDL_SCANCODE_U);
	InsertLowercaseCharacter('v', SDL_SCANCODE_V);
	InsertLowercaseCharacter('w', SDL_SCANCODE_W);
	InsertLowercaseCharacter('x', SDL_SCANCODE_X);
	InsertLowercaseCharacter('y', SDL_SCANCODE_Y);
	InsertLowercaseCharacter('z', SDL_SCANCODE_Z);

	InsertShiftCharacter('{', SDL_SCANCODE_LEFTBRACKET);
	InsertShiftCharacter('|', SDL_SCANCODE_BACKSLASH);
	InsertShiftCharacter('}', SDL_SCANCODE_RIGHTBRACKET);
	InsertShiftCharacter('~', SDL_SCANCODE_GRAVE);
}


int VIPR_Emulator::Application::GetReturnCode() const
{
	return retcode;
}

void VIPR_Emulator::Application::DrawCurrentMenu()
{
	MainRenderer.ClearSecondaryFramebuffer();
	if (CurrentMenu != nullptr)
	{
		GUI::DrawElement(MainRenderer, *CurrentMenu);
	}
}

void VIPR_Emulator::Application::SetOperationMode(OperationMode mode)
{
	current_operation_mode = mode;
	switch (current_operation_mode)
	{
		case OperationMode::Menu:
		{
			key_down_callback = menu_key_down;
			key_up_callback = menu_key_up;
			break;
		}
		case OperationMode::Input:
		{
			key_down_callback = input_key_down;
			key_up_callback = input_key_up;
			break;
		}
		case OperationMode::Machine:
		{
			key_down_callback = machine_key_down;
			key_up_callback = machine_key_up;
			break;
		}
	}
}

void VIPR_Emulator::Application::ConstructMenus()
{
	constexpr GUI::ColorData main_menu_item_color { 0xA0, 0xA0, 0xA0 };
	constexpr GUI::ColorData main_menu_item_select_color { 0xA0, 0x00, 0x00 };
	constexpr GUI::ColorData main_menu_item_disabled_color { 0x40, 0x40, 0x40 };
	MainMenu.x = 152;
	MainMenu.y = 30;
	MainMenu.current_menu_item = 0;
	MainMenu.hidden = false;
	MainMenu.on_up = main_menu_up;
	MainMenu.on_down = main_menu_down;
	MainMenu.on_left = main_menu_left;
	MainMenu.on_right = main_menu_right;
	MainMenu.on_activate = main_menu_activate;
	std::ostringstream title_stream;
	title_stream << "VIPR Emulator V" << version.major << '.' << version.minor << " - A COSMAC VIP Emulator";
	MainMenu.element_list.push_back(GUI::ElementData { GUI::ElementType::Text, GUI::Text { title_stream.str(), 0, 0, GUI::ColorData { 0xC0, 0xC0, 0xC0 }, false } });
	MainMenu.element_list.push_back(GUI::ElementData { GUI::ElementType::Text, GUI::Text { "By Joshua Moss", 96, 10, GUI::ColorData { 0x80, 0x80, 0x80 }, false } });
	MainMenu.element_list.push_back(GUI::ElementData { GUI::ElementType::Toggle, GUI::Toggle { "Machine Power", 64, 50, main_menu_item_color, main_menu_item_select_color, GUI::ColorData { 0xFF, 0xFF, 0xFF }, true, false, false, nullptr } });
	MainMenu.element_list.push_back(GUI::ElementData { GUI::ElementType::Button, GUI::Button { "Switch to Machine", 64, 60, main_menu_item_color, main_menu_item_select_color, main_menu_item_disabled_color, false, true, false, nullptr } });
	MainMenu.element_list.push_back(GUI::ElementData { GUI::ElementType::Button, GUI::Button { "Machine Options", 64, 70, main_menu_item_color, main_menu_item_select_color, main_menu_item_disabled_color, false, false, false, nullptr } });
	MainMenu.element_list.push_back(GUI::ElementData { GUI::ElementType::Button, GUI::Button { "Machine Memory Transfer", 64, 80, main_menu_item_color, main_menu_item_select_color, main_menu_item_disabled_color, false, true, false, nullptr } });
	MainMenu.element_list.push_back(GUI::ElementData { GUI::ElementType::Button, GUI::Button { "Emulator Options", 64, 90, main_menu_item_color, main_menu_item_select_color, main_menu_item_disabled_color, false, false, false, nullptr } });
	MainMenu.element_list.push_back(GUI::ElementData { GUI::ElementType::Button, GUI::Button { "Exit Emulator", 64, 100, main_menu_item_color, main_menu_item_select_color, main_menu_item_disabled_color, false, false, false, nullptr } });

	MachineOptionsMenu.x = 152;
	MachineOptionsMenu.y = 30;
	MachineOptionsMenu.current_menu_item = 0;
	MachineOptionsMenu.hidden = false;
	MachineOptionsMenu.on_up = machine_options_up;
	MachineOptionsMenu.on_down = machine_options_down;
	MachineOptionsMenu.on_left = machine_options_left;
	MachineOptionsMenu.on_right = machine_options_right;
	MachineOptionsMenu.on_activate = machine_options_activate;
	MachineOptionsMenu.element_list.push_back(GUI::ElementData { GUI::ElementType::Text, GUI::Text { "Machine Options", 88, 0, GUI::ColorData { 0xC0, 0xC0, 0xC0 }, false } });
	MachineOptionsMenu.element_list.push_back(GUI::ElementData { GUI::ElementType::Value, GUI::Value { "RAM (In KB)", "", 0, 50, main_menu_item_color, main_menu_item_select_color, GUI::ColorData { 0xFF, 0xFF, 0xFF }, GUI::ValueBaseType::Decimal, 2, 1, 32, 0, true, false, false, machine_options_ram_in_kb_input_complete } });
	MachineOptionsMenu.element_list.push_back(GUI::ElementData { GUI::ElementType::Input, GUI::Input { "ROM File", "", "", 0, 60, main_menu_item_color, main_menu_item_select_color, GUI::ColorData { 0xFF, 0xFF, 0xFF }, 0, 32, 0, 0, false, false, false, machine_options_rom_file_input_complete } });
	MachineOptionsMenu.element_list.push_back(GUI::ElementData { GUI::ElementType::Button, GUI::Button { "Load ROM", 0, 70, main_menu_item_color, main_menu_item_select_color, main_menu_item_disabled_color, false, true, false, nullptr } });
	MachineOptionsMenu.element_list.push_back(GUI::ElementData { GUI::ElementType::Button, GUI::Button { "Expansion Board Options", 0, 80, main_menu_item_color, main_menu_item_select_color, main_menu_item_disabled_color, false, false, false, nullptr } });
	MachineOptionsMenu.element_list.push_back(GUI::ElementData { GUI::ElementType::Status, GUI::Status { "ROM Status", "None", 0, 130, main_menu_item_color, GUI::ColorData { 0x40, 0x40, 0x40 }, false } });
	MachineOptionsMenu.element_list.push_back(GUI::ElementData { GUI::ElementType::Button, GUI::Button { "Return to Main Menu", 64, 180, main_menu_item_color, main_menu_item_select_color, main_menu_item_disabled_color, false, false, false, nullptr } });

	ExpansionBoardOptionsMenu.x = 30;
	ExpansionBoardOptionsMenu.y = 30;
	ExpansionBoardOptionsMenu.current_menu_item = 0;
	ExpansionBoardOptionsMenu.hidden = false;
	ExpansionBoardOptionsMenu.on_up = expansion_board_options_up;
	ExpansionBoardOptionsMenu.on_down = expansion_board_options_down;
	ExpansionBoardOptionsMenu.on_left = expansion_board_options_left;
	ExpansionBoardOptionsMenu.on_right = expansion_board_options_right;
	ExpansionBoardOptionsMenu.on_activate = expansion_board_options_activate;
	ExpansionBoardOptionsMenu.element_list.push_back(GUI::ElementData { GUI::ElementType::Text, GUI::Text { "Expansion Board Options", 184, 0, GUI::ColorData { 0xC0, 0xC0, 0xC0 }, false } });
	ExpansionBoardOptionsMenu.element_list.push_back(GUI::ElementData { GUI::ElementType::Toggle, GUI::Toggle { "VP-585 Expansion Keypad Interface", 0, 30, main_menu_item_color, main_menu_item_select_color, GUI::ColorData { 0xFF, 0xFF, 0xFF }, true, false, false, nullptr } });
	ExpansionBoardOptionsMenu.element_list.push_back(GUI::ElementData { GUI::ElementType::Toggle, GUI::Toggle { "VP-590 Color Board", 0, 40, main_menu_item_color, main_menu_item_select_color, GUI::ColorData { 0xFF, 0xFF, 0xFF }, false, false, false, nullptr } });
	ExpansionBoardOptionsMenu.element_list.push_back(GUI::ElementData { GUI::ElementType::Toggle, GUI::Toggle { "VP-595 Simple Sound Board", 0, 50, main_menu_item_color, main_menu_item_select_color, GUI::ColorData { 0xFF, 0xFF, 0xFF }, false, false, false, nullptr } });
	ExpansionBoardOptionsMenu.element_list.push_back(GUI::ElementData { GUI::ElementType::Button, GUI::Button { "Return to Machine Options", 184, 200, main_menu_item_color, main_menu_item_select_color, main_menu_item_disabled_color, false, false, false, nullptr } });

	MachineMemoryTransferMenu.x = 136;
	MachineMemoryTransferMenu.y = 30;
	MachineMemoryTransferMenu.current_menu_item = 0;
	MachineMemoryTransferMenu.hidden = false;
	MachineMemoryTransferMenu.on_up = machine_memory_transfer_up;
	MachineMemoryTransferMenu.on_down = machine_memory_transfer_down;
	MachineMemoryTransferMenu.on_left = machine_memory_transfer_left;
	MachineMemoryTransferMenu.on_right = machine_memory_transfer_right;
	MachineMemoryTransferMenu.on_activate = machine_memory_transfer_activate;
	MachineMemoryTransferMenu.element_list.push_back(GUI::ElementData { GUI::ElementType::Text, GUI::Text { "Machine Memory Transfer", 72, 0, GUI::ColorData { 0xC0, 0xC0, 0xC0 }, false } });
	MachineMemoryTransferMenu.element_list.push_back(GUI::ElementData { GUI::ElementType::Input, GUI::Input { "Memory File", "", "", 0, 50, main_menu_item_color, main_menu_item_select_color, GUI::ColorData { 0xFF, 0xFF, 0xFF }, 0, 32, 0, 0, true, false, false, machine_memory_transfer_memory_file_input_complete } });
	MachineMemoryTransferMenu.element_list.push_back(GUI::ElementData { GUI::ElementType::MultiChoice, GUI::MultiChoice { "Transfer Type", 0, 60, main_menu_item_color, main_menu_item_select_color, GUI::ColorData { 0xFF, 0xFF, 0xFF }, 0, std::vector<std::string> { "Load", "Store" }, false, false } });
	MachineMemoryTransferMenu.element_list.push_back(GUI::ElementData { GUI::ElementType::Value, GUI::Value { "Start Address", "", 0, 70, main_menu_item_color, main_menu_item_select_color, GUI::ColorData { 0xFF, 0xFF, 0xFF }, GUI::ValueBaseType::Hexadecimal, 0x0000, 0x0000, 0x7FFF, 0, false, false, false, machine_memory_transfer_start_address_input_complete } });
	MachineMemoryTransferMenu.element_list.push_back(GUI::ElementData { GUI::ElementType::Value, GUI::Value { "Size", "", 0, 80, main_menu_item_color, main_menu_item_select_color, GUI::ColorData { 0xFF, 0xFF, 0xFF }, GUI::ValueBaseType::Decimal, 512, 1, 32768, 0, false, false, false, machine_memory_transfer_size_input_complete } });
	MachineMemoryTransferMenu.element_list.push_back(GUI::ElementData { GUI::ElementType::Button, GUI::Button { "Transfer", 0, 90, main_menu_item_color, main_menu_item_select_color, main_menu_item_disabled_color, false, true, false } });
	MachineMemoryTransferMenu.element_list.push_back(GUI::ElementData { GUI::ElementType::Status, GUI::Status { "Transfer Status", "None", 0, 130, main_menu_item_color, GUI::ColorData { 0x40, 0x40, 0x40 }, false } });
	MachineMemoryTransferMenu.element_list.push_back(GUI::ElementData { GUI::ElementType::Button, GUI::Button { "Return to Main Menu", 80, 180, main_menu_item_color, main_menu_item_select_color, main_menu_item_disabled_color, false, false, false, nullptr } });

	EmulatorOptionsMenu.x = 132;
	EmulatorOptionsMenu.y = 30;
	EmulatorOptionsMenu.current_menu_item = 0;
	EmulatorOptionsMenu.hidden = false;
	EmulatorOptionsMenu.on_up = emulator_options_up;
	EmulatorOptionsMenu.on_down = emulator_options_down;
	EmulatorOptionsMenu.on_left = emulator_options_left;
	EmulatorOptionsMenu.on_right = emulator_options_right;
	EmulatorOptionsMenu.on_activate = emulator_options_activate;
	EmulatorOptionsMenu.element_list.push_back(GUI::ElementData { GUI::ElementType::Text, GUI::Text { "Emulator Options", 108, 0, GUI::ColorData { 0xC0, 0xC0, 0xC0 }, false } });
	std::vector<std::string> OutputAudioDeviceList(SDL_GetNumAudioDevices(0));
	for (size_t i = 0; i < OutputAudioDeviceList.size(); ++i)
	{
		OutputAudioDeviceList[i] = SDL_GetAudioDeviceName(i, 0);
	}
	EmulatorOptionsMenu.element_list.push_back(GUI::ElementData { GUI::ElementType::MultiChoice, GUI::MultiChoice { "Output Audio Device", 0, 50, main_menu_item_color, main_menu_item_select_color, GUI::ColorData { 0xFF, 0xFF, 0xFF }, 0, std::move(OutputAudioDeviceList), true, false } });
	EmulatorOptionsMenu.element_list.push_back(GUI::ElementData { GUI::ElementType::Value, GUI::Value { "Main Volume", "", 0, 60, main_menu_item_color, main_menu_item_select_color, GUI::ColorData { 0xFF, 0xFF, 0xFF }, GUI::ValueBaseType::Decimal, 50, 0, 100, 0, false, false, false, nullptr } });
	EmulatorOptionsMenu.element_list.push_back(GUI::ElementData { GUI::ElementType::Button, GUI::Button { "Return to Main Menu", 114, 180, main_menu_item_color, main_menu_item_select_color, main_menu_item_disabled_color, false, false, false, nullptr } });
}

void VIPR_Emulator::menu_key_down(Application *app, SDL_Scancode scancode, uint16_t modifiers)
{
	if (app == nullptr)
	{
		return;
	}
	switch (scancode)
	{
		case SDL_SCANCODE_UP:
		{
			if (app->CurrentMenu != nullptr)
			{
				if (app->CurrentMenu->on_up != nullptr)
				{
					app->CurrentMenu->on_up(*app->CurrentMenu, app);
				}
			}
			break;
		}
		case SDL_SCANCODE_DOWN:
		{
			if (app->CurrentMenu != nullptr)
			{
				if (app->CurrentMenu->on_down != nullptr)
				{
					app->CurrentMenu->on_down(*app->CurrentMenu, app);
				}
			}
			break;
		}
		case SDL_SCANCODE_LEFT:
		{
			if (app->CurrentMenu != nullptr)
			{
				if (app->CurrentMenu->on_left != nullptr)
				{
					app->CurrentMenu->on_left(*app->CurrentMenu, app);
				}
			}
			break;
		}
		case SDL_SCANCODE_RIGHT:
		{
			if (app->CurrentMenu != nullptr)
			{
				if (app->CurrentMenu->on_right != nullptr)
				{
					app->CurrentMenu->on_right(*app->CurrentMenu, app);
				}
			}
			break;
		}
		case SDL_SCANCODE_RETURN:
		{
			if (app->CurrentMenu != nullptr)
			{
				if (app->CurrentMenu->on_activate != nullptr)
				{
					app->CurrentMenu->on_activate(*app->CurrentMenu, app);
				}
			}
			break;
		}
	}
}

void VIPR_Emulator::menu_key_up(Application *app, SDL_Scancode scancode, uint16_t modifiers)
{
	if (app == nullptr)
	{
		return;
	}
}

void VIPR_Emulator::input_key_down(VIPR_Emulator::Application *app, SDL_Scancode scancode, uint16_t modifiers)
{
	switch (scancode)
	{
		case SDL_SCANCODE_SPACE:
		case SDL_SCANCODE_APOSTROPHE:
		case SDL_SCANCODE_COMMA:
		case SDL_SCANCODE_MINUS:
		case SDL_SCANCODE_PERIOD:
		case SDL_SCANCODE_SLASH:
		case SDL_SCANCODE_0:
		case SDL_SCANCODE_1:
		case SDL_SCANCODE_2:
		case SDL_SCANCODE_3:
		case SDL_SCANCODE_4:
		case SDL_SCANCODE_5:
		case SDL_SCANCODE_6:
		case SDL_SCANCODE_7:
		case SDL_SCANCODE_8:
		case SDL_SCANCODE_9:
		case SDL_SCANCODE_SEMICOLON:
		case SDL_SCANCODE_EQUALS:
		case SDL_SCANCODE_A:
		case SDL_SCANCODE_B:
		case SDL_SCANCODE_C:
		case SDL_SCANCODE_D:
		case SDL_SCANCODE_E:
		case SDL_SCANCODE_F:
		case SDL_SCANCODE_G:
		case SDL_SCANCODE_H:
		case SDL_SCANCODE_I:
		case SDL_SCANCODE_J:
		case SDL_SCANCODE_K:
		case SDL_SCANCODE_L:
		case SDL_SCANCODE_M:
		case SDL_SCANCODE_N:
		case SDL_SCANCODE_O:
		case SDL_SCANCODE_P:
		case SDL_SCANCODE_Q:
		case SDL_SCANCODE_R:
		case SDL_SCANCODE_S:
		case SDL_SCANCODE_T:
		case SDL_SCANCODE_U:
		case SDL_SCANCODE_V:
		case SDL_SCANCODE_W:
		case SDL_SCANCODE_X:
		case SDL_SCANCODE_Y:
		case SDL_SCANCODE_Z:
		case SDL_SCANCODE_LEFTBRACKET:
		case SDL_SCANCODE_BACKSLASH:
		case SDL_SCANCODE_RIGHTBRACKET:
		case SDL_SCANCODE_GRAVE:
		{
			switch (app->InputFocus->type)
			{
				case GUI::ElementType::Input:
				{
					GUI::Input *CurrentInput = std::get_if<GUI::Input>(&app->InputFocus->element);
					uint16_t input_count = CurrentInput->input.size();
					if (input_count < CurrentInput->max_length || CurrentInput->max_length == 0)
					{
						for (auto [k, v] : app->Printable_KeyMap)
						{
							if (v.scancode == scancode && v.modifiers == modifiers)
							{
								if (CurrentInput->cursor_pos < input_count)
								{
									std::string tmp_str = "";
									for (uint16_t i = 0; i < CurrentInput->cursor_pos; ++i)
									{
										tmp_str += CurrentInput->input[i];
									}
									tmp_str += k;
									for (uint16_t i = CurrentInput->cursor_pos; i < input_count; ++i)
									{
										tmp_str += CurrentInput->input[i];
									}
									CurrentInput->input = std::move(tmp_str);
								}
								else
								{
									CurrentInput->input += k;
								}
								++CurrentInput->cursor_pos;
								if (CurrentInput->cursor_pos >= CurrentInput->display_start + CurrentInput->max_display && CurrentInput->display_start < CurrentInput->input.size() + 1)
								{
									++CurrentInput->display_start;
								}
								break;
							}
						}
					}
					break;
				}
				case GUI::ElementType::Value:
				{
					GUI::Value *CurrentValue = std::get_if<GUI::Value>(&app->InputFocus->element);
					uint16_t input_count = CurrentValue->input.size();
					if (input_count < 10)
					{
						auto KeyCheck = [&CurrentValue](std::pair<char, ScancodeModData> printable_key)
						{
							switch (CurrentValue->base)
							{
								case GUI::ValueBaseType::Decimal:
								{
									if ((printable_key.first >= '0' && printable_key.first <= '9') || printable_key.first == '+' || printable_key.first == '-')
									{
										return true;
									}
									break;
								}
								case GUI::ValueBaseType::Hexadecimal:
								{
									char u_key = toupper(printable_key.first);
									if ((printable_key.first >= '0' && printable_key.first <= '9') || (u_key >= 'A' && u_key <= 'F'))
									{
										return true;
									}
									break;
								}
							}
							return false;
						};
						for (auto [k, v] : app->Printable_KeyMap | std::ranges::views::filter(KeyCheck))
						{
							if (v.scancode == scancode && v.modifiers == modifiers)
							{
								if (CurrentValue->cursor_pos < input_count)
								{
									std::string tmp_str = "";
									for (uint16_t i = 0; i < CurrentValue->cursor_pos; ++i)
									{
										tmp_str += CurrentValue->input[i];
									}
									tmp_str += k;
									for (uint16_t i = CurrentValue->cursor_pos; i < input_count; ++i)
									{
										tmp_str += CurrentValue->input[i];
									}
									CurrentValue->input = std::move(tmp_str);
								}
								else
								{
									CurrentValue->input += k;
								}
								++CurrentValue->cursor_pos;
								break;
							}
						}
					}
					break;
				}
			}
			app->DrawCurrentMenu();
			break;
		}
		case SDL_SCANCODE_LEFT:
		{
			switch (app->InputFocus->type)
			{
				case GUI::ElementType::Input:
				{
					GUI::Input *CurrentInput = std::get_if<GUI::Input>(&app->InputFocus->element);
					if (CurrentInput->cursor_pos > 0)
					{
						--CurrentInput->cursor_pos;
						if (CurrentInput->cursor_pos < CurrentInput->display_start + 1 && CurrentInput->display_start > 0)
						{
							--CurrentInput->display_start;
						}
					}
					break;
				}
				case GUI::ElementType::Value:
				{
					GUI::Value *CurrentValue = std::get_if<GUI::Value>(&app->InputFocus->element);
					if (CurrentValue->cursor_pos > 0)
					{
						--CurrentValue->cursor_pos;
					}
					break;
				}
			}
			app->DrawCurrentMenu();
			break;
		}
		case SDL_SCANCODE_RIGHT:
		{
			switch (app->InputFocus->type)
			{
				case GUI::ElementType::Input:
				{
					GUI::Input *CurrentInput = std::get_if<GUI::Input>(&app->InputFocus->element);
					if (CurrentInput->cursor_pos < CurrentInput->input.size())
					{
						++CurrentInput->cursor_pos;
						if (CurrentInput->cursor_pos >= CurrentInput->display_start + CurrentInput->max_display && CurrentInput->display_start < CurrentInput->input.size() + 1)
						{
							++CurrentInput->display_start;
						}
					}
					break;
				}
				case GUI::ElementType::Value:
				{
					GUI::Value *CurrentValue = std::get_if<GUI::Value>(&app->InputFocus->element);
					if (CurrentValue->cursor_pos < CurrentValue->input.size())
					{
						++CurrentValue->cursor_pos;
					}
					break;
				}
			}
			app->DrawCurrentMenu();
			break;
		}
		case SDL_SCANCODE_ESCAPE:
		{
			switch (app->InputFocus->type)
			{
				case GUI::ElementType::Input:
				{
					GUI::Input *CurrentInput = std::get_if<GUI::Input>(&app->InputFocus->element);
					app->InputFocus = nullptr;
					CurrentInput->focus = false;
					app->SetOperationMode(OperationMode::Menu);
					break;
				}
				case GUI::ElementType::Value:
				{
					GUI::Value *CurrentValue = std::get_if<GUI::Value>(&app->InputFocus->element);
					app->InputFocus = nullptr;
					CurrentValue->focus = false;
					app->SetOperationMode(OperationMode::Menu);
					break;
				}
			}
			app->DrawCurrentMenu();
			break;
		}
		case SDL_SCANCODE_RETURN:
		{
			switch (app->InputFocus->type)
			{
				case GUI::ElementType::Input:
				{
					GUI::Input *CurrentInput = std::get_if<GUI::Input>(&app->InputFocus->element);
					CurrentInput->stored_input = std::move(CurrentInput->input);
					if (CurrentInput->on_input_complete != nullptr)
					{
						CurrentInput->on_input_complete(*CurrentInput, app);
					}
					app->InputFocus = nullptr;
					CurrentInput->focus = false;
					app->SetOperationMode(OperationMode::Menu);
					break;
				}
				case GUI::ElementType::Value:
				{
					GUI::Value *CurrentValue = std::get_if<GUI::Value>(&app->InputFocus->element);
					uint16_t input_count = CurrentValue->input.size();
					if (input_count > 0)
					{
						std::istringstream current_stream(CurrentValue->input);
						if (CurrentValue->base == GUI::ValueBaseType::Hexadecimal)
						{
							current_stream >> std::hex;
						}
						current_stream >> CurrentValue->value;
						if (CurrentValue->value < CurrentValue->min)
						{
							CurrentValue->value = CurrentValue->min;
						}
						else if (CurrentValue->value > CurrentValue->max)
						{
							CurrentValue->value = CurrentValue->max;
						}
						if (CurrentValue->on_input_complete != nullptr)
						{
							CurrentValue->on_input_complete(*CurrentValue, app);
						}
					}
					app->InputFocus = nullptr;
					CurrentValue->focus = false;
					app->SetOperationMode(OperationMode::Menu);
					break;
				}
			}
			app->DrawCurrentMenu();
			break;
		}
		case SDL_SCANCODE_BACKSPACE:
		{
			switch (app->InputFocus->type)
			{
				case GUI::ElementType::Input:
				{
					GUI::Input *CurrentInput = std::get_if<GUI::Input>(&app->InputFocus->element);
					uint16_t input_count = CurrentInput->input.size();
					if (input_count > 0 && (CurrentInput->cursor_pos >= CurrentInput->display_start))
					{
						std::string tmp_str = "";
						for (uint16_t i = 0; i < input_count; ++i)
						{
							if (i != CurrentInput->cursor_pos - 1)
							{
								tmp_str += CurrentInput->input[i];
							}
						}
						CurrentInput->input = std::move(tmp_str);
						if (CurrentInput->cursor_pos > 0)
						{
							--CurrentInput->cursor_pos;
							if (CurrentInput->cursor_pos < CurrentInput->display_start + 1 && CurrentInput->display_start > 0)
							{
								--CurrentInput->display_start;
							}
						}
					}
					break;
				}
				case GUI::ElementType::Value:
				{
					GUI::Value *CurrentValue = std::get_if<GUI::Value>(&app->InputFocus->element);
					uint16_t input_count = CurrentValue->input.size();
					if (input_count > 0 && CurrentValue->cursor_pos > 0)
					{
						std::string tmp_str = "";
						for (uint16_t i = 0; i < input_count; ++i)
						{
							if (i != CurrentValue->cursor_pos - 1)
							{
								tmp_str += CurrentValue->input[i];
							}
						}
						CurrentValue->input = std::move(tmp_str);
						--CurrentValue->cursor_pos;
					}
					break;
				}
			}
			app->DrawCurrentMenu();
			break;
		}
	}
}

void VIPR_Emulator::input_key_up(VIPR_Emulator::Application *app, SDL_Scancode scancode, uint16_t modifiers)
{
}

void VIPR_Emulator::machine_key_down(VIPR_Emulator::Application *app, SDL_Scancode scancode, uint16_t modifiers)
{
	if (app == nullptr)
	{
		return;
	}
	bool key_found = false;
	for (auto [k, v] : app->Hex_KeyMap)
	{
		if (scancode == v)
		{
			key_found = true;
			app->current_hex_key = static_cast<uint8_t>(k);
			app->System.IssueHexKeyPress(app->current_hex_key, 0);
			break;
		}
	}
	if (key_found)
	{
		return;
	}
	for (auto [k, v] : app->Hex_KeyMap_2)
	{
		if (scancode == v)
		{
			key_found = true;
			app->current_hex_key = static_cast<uint8_t>(k);
			app->System.IssueHexKeyPress(app->current_hex_key, 1);
			break;
		}
	}
	if (key_found)
	{
		return;
	}
	if (scancode == SDL_SCANCODE_RETURN)
	{
		bool run = app->System.IsRunning() ? false : true;
		app->System.SetRunSwitch(run);
		if (app->System.IsRunning())
		{
			SDL_SetWindowTitle(app->MainWindow.get(), "VIPR Emulator (Running)");
		}
		else
		{
			SDL_SetWindowTitle(app->MainWindow.get(), "VIPR Emulator");
		}
	}
	else if (scancode == SDL_SCANCODE_ESCAPE)
	{
		app->SetOperationMode(OperationMode::Menu);
		app->MainRenderer.SetDisplayType(DisplayType::Emulator);
	}
}

void VIPR_Emulator::machine_key_up(VIPR_Emulator::Application *app, SDL_Scancode scancode, uint16_t modifiers)
{
	if (app == nullptr)
	{
		return;
	}
	bool key_found = false;
	for (auto [k, v] : app->Hex_KeyMap)
	{
		if (scancode == v)
		{
			if (app->current_hex_key == static_cast<uint8_t>(k))
			{
				app->System.IssueHexKeyRelease(0);
				key_found = true;
			}
			break;
		}
	}
	if (key_found)
	{
		return;
	}
	for (auto [k, v] : app->Hex_KeyMap_2)
	{
		if (scancode == v)
		{
			if (app->current_hex_key == static_cast<uint8_t>(k))
			{
				app->System.IssueHexKeyRelease(1);
			}
			break;
		}
	}
}

void VIPR_Emulator::main_menu_down(VIPR_Emulator::GUI::Menu &obj, void *userdata)
{
	Application *app = static_cast<Application *>(userdata);
	GUI::Toggle *MachinePower = std::get_if<GUI::Toggle>(&obj.element_list[2].element);
	GUI::Button *SwitchToMachine = std::get_if<GUI::Button>(&obj.element_list[3].element);
	GUI::Button *MachineOptions = std::get_if<GUI::Button>(&obj.element_list[4].element);
	GUI::Button *MachineMemoryTransfer = std::get_if<GUI::Button>(&obj.element_list[5].element);
	GUI::Button *EmulatorOptions = std::get_if<GUI::Button>(&obj.element_list[6].element);
	GUI::Button *ExitEmulator = std::get_if<GUI::Button>(&obj.element_list[7].element);
	switch (obj.current_menu_item)
	{
		case 0:
		{
			MachinePower->select = false;
			break;
		}
		case 1:
		{
			SwitchToMachine->select = false;
			break;
		}
		case 2:
		{
			MachineOptions->select = false;
			break;
		}
		case 3:
		{
			MachineMemoryTransfer->select = false;
			break;
		}
		case 4:
		{
			EmulatorOptions->select = false;
			break;
		}
		case 5:
		{
			ExitEmulator->select = false;
			break;
		}
	}
	obj.current_menu_item = (obj.current_menu_item == 5) ? 0 : obj.current_menu_item + 1;
	bool selected = false;
	while (!selected)
	{
		switch (obj.current_menu_item)
		{
			case 0:
			{
				MachinePower->select = true;
				selected = true;
				break;
			}
			case 1:
			{
				if (!SwitchToMachine->disabled)
				{
					SwitchToMachine->select = true;
					selected = true;
				}
				else
				{
					++obj.current_menu_item;
				}
				break;
			}
			case 2:
			{
				if (!MachineOptions->disabled)
				{
					MachineOptions->select = true;
					selected = true;
				}
				else
				{
					++obj.current_menu_item;
				}
				break;
			}
			case 3:
			{
				if (!MachineMemoryTransfer->disabled)
				{
					MachineMemoryTransfer->select = true;
					selected = true;
				}
				else
				{
					++obj.current_menu_item;
				}
				break;
			}
			case 4:
			{
				if (!EmulatorOptions->disabled)
				{
					EmulatorOptions->select = true;
					selected = true;
				}
				else
				{
					++obj.current_menu_item;
				}
				break;
			}
			case 5:
			{
				if (!ExitEmulator->disabled)
				{
					ExitEmulator->select = true;
					selected = true;
				}
				else
				{
					obj.current_menu_item = 0;
				}
				break;
			}
		}
	}
	app->DrawCurrentMenu();
}

void VIPR_Emulator::main_menu_up(VIPR_Emulator::GUI::Menu &obj, void *userdata)
{
	Application *app = static_cast<Application *>(userdata);
	GUI::Toggle *MachinePower = std::get_if<GUI::Toggle>(&obj.element_list[2].element);
	GUI::Button *SwitchToMachine = std::get_if<GUI::Button>(&obj.element_list[3].element);
	GUI::Button *MachineOptions = std::get_if<GUI::Button>(&obj.element_list[4].element);
	GUI::Button *MachineMemoryTransfer = std::get_if<GUI::Button>(&obj.element_list[5].element);
	GUI::Button *EmulatorOptions = std::get_if<GUI::Button>(&obj.element_list[6].element);
	GUI::Button *ExitEmulator = std::get_if<GUI::Button>(&obj.element_list[7].element);
	switch (obj.current_menu_item)
	{
		case 0:
		{
			MachinePower->select = false;
			break;
		}
		case 1:
		{
			SwitchToMachine->select = false;
			break;
		}
		case 2:
		{
			MachineOptions->select = false;
			break;
		}
		case 3:
		{
			MachineMemoryTransfer->select = false;
			break;
		}
		case 4:
		{
			EmulatorOptions->select = false;
			break;
		}
		case 5:
		{
			ExitEmulator->select = false;
			break;
		}
	}
	obj.current_menu_item = (obj.current_menu_item == 0) ? 5 : obj.current_menu_item - 1;
	bool selected = false;
	while (!selected)
	{
		switch (obj.current_menu_item)
		{
			case 0:
			{
				MachinePower->select = true;
				selected = true;
				break;
			}
			case 1:
			{
				if (!SwitchToMachine->disabled)
				{
					SwitchToMachine->select = true;
					selected = true;
				}
				else
				{
					--obj.current_menu_item;
				}
				break;
			}
			case 2:
			{
				if (!MachineOptions->disabled)
				{
					MachineOptions->select = true;
					selected = true;
				}
				else
				{
					--obj.current_menu_item;
				}
				break;
			}
			case 3:
			{
				if (!MachineMemoryTransfer->disabled)
				{
					MachineMemoryTransfer->select = true;
					selected = true;
				}
				else
				{
					--obj.current_menu_item;
				}
				break;
			}
			case 4:
			{
				if (!EmulatorOptions->disabled)
				{
					EmulatorOptions->select = true;
					selected = true;
				}
				else
				{
					--obj.current_menu_item;
				}
				break;
			}
			case 5:
			{
				if (!ExitEmulator->disabled)
				{
					ExitEmulator->select = true;
					selected = true;
				}
				else
				{
					--obj.current_menu_item;
				}
				break;
			}
		}
	}
	app->DrawCurrentMenu();
}

void VIPR_Emulator::main_menu_left(VIPR_Emulator::GUI::Menu &obj, void *userdata)
{
	Application *app = static_cast<Application *>(userdata);
	GUI::Toggle *MachinePower = std::get_if<GUI::Toggle>(&obj.element_list[2].element);
	GUI::Button *SwitchToMachine = std::get_if<GUI::Button>(&obj.element_list[3].element);
	GUI::Button *MachineOptions = std::get_if<GUI::Button>(&obj.element_list[4].element);
	GUI::Button *MachineMemoryTransfer = std::get_if<GUI::Button>(&obj.element_list[5].element);
	switch (obj.current_menu_item)
	{
		case 0:
		{
			MachinePower->toggle = (MachinePower->toggle) ? false : true;
			if (!MachinePower->toggle)
			{
				app->System.Reset();
				SDL_SetWindowTitle(app->MainWindow.get(), "VIPR Emulator");
				SwitchToMachine->disabled = true;
				MachineOptions->disabled = false;
				MachineMemoryTransfer->disabled = true;
			}
			else
			{
				SwitchToMachine->disabled = false;
				MachineOptions->disabled = true;
				MachineMemoryTransfer->disabled = false;
			}
			break;
		}
	}
	app->DrawCurrentMenu();
}

void VIPR_Emulator::main_menu_right(VIPR_Emulator::GUI::Menu &obj, void *userdata)
{
	Application *app = static_cast<Application *>(userdata);
	GUI::Toggle *MachinePower = std::get_if<GUI::Toggle>(&obj.element_list[2].element);
	GUI::Button *SwitchToMachine = std::get_if<GUI::Button>(&obj.element_list[3].element);
	GUI::Button *MachineOptions = std::get_if<GUI::Button>(&obj.element_list[4].element);
	GUI::Button *MachineMemoryTransfer = std::get_if<GUI::Button>(&obj.element_list[5].element);
	switch (obj.current_menu_item)
	{
		case 0:
		{
			MachinePower->toggle = (MachinePower->toggle) ? false : true;
			if (!MachinePower->toggle)
			{
				app->System.Reset();
				SDL_SetWindowTitle(app->MainWindow.get(), "VIPR Emulator");
				SwitchToMachine->disabled = true;
				MachineOptions->disabled = false;
				MachineMemoryTransfer->disabled = true;
			}
			else
			{
				SwitchToMachine->disabled = false;
				MachineOptions->disabled = true;
				MachineMemoryTransfer->disabled = false;
			}
			break;
		}
	}
	app->DrawCurrentMenu();
}

void VIPR_Emulator::main_menu_activate(VIPR_Emulator::GUI::Menu &obj, void *userdata)
{
	Application *app = static_cast<Application *>(userdata);
	GUI::Toggle *MachinePower = std::get_if<GUI::Toggle>(&obj.element_list[2].element);
	GUI::Button *SwitchToMachine = std::get_if<GUI::Button>(&obj.element_list[3].element);
	GUI::Button *MachineOptions = std::get_if<GUI::Button>(&obj.element_list[4].element);
	GUI::Button *MachineMemoryTransfer = std::get_if<GUI::Button>(&obj.element_list[5].element);
	GUI::Button *EmulatorOptions = std::get_if<GUI::Button>(&obj.element_list[6].element);
	GUI::Button *ExitEmulator = std::get_if<GUI::Button>(&obj.element_list[7].element);
	switch (obj.current_menu_item)
	{
		case 0:
		{
			MachinePower->toggle = (MachinePower->toggle) ? false : true;
			if (!MachinePower->toggle)
			{
				app->System.Reset();
				SDL_SetWindowTitle(app->MainWindow.get(), "VIPR Emulator");
				SwitchToMachine->disabled = true;
				MachineOptions->disabled = false;
				MachineMemoryTransfer->disabled = true;
			}
			else
			{
				SwitchToMachine->disabled = false;
				MachineOptions->disabled = true;
				MachineMemoryTransfer->disabled = false;
			}
			break;
		}
		case 1:
		{
			app->SetOperationMode(OperationMode::Machine);
			app->MainRenderer.SetDisplayType(DisplayType::Machine);
			app->System.SetCPUCycleTimePoint(std::chrono::high_resolution_clock::now());
			break;
		}
		case 2:
		{
			app->CurrentMenu = &app->MachineOptionsMenu;
			break;
		}
		case 3:
		{
			app->CurrentMenu = &app->MachineMemoryTransferMenu;
			break;
		}
		case 4:
		{
			app->CurrentMenu = &app->EmulatorOptionsMenu;
			break;
		}
		case 5:
		{
			app->exit = true;
			break;
		}
	}
	app->DrawCurrentMenu();
}

void VIPR_Emulator::machine_options_up(VIPR_Emulator::GUI::Menu &obj, void *userdata)
{
	Application *app = static_cast<Application *>(userdata);
	GUI::Value *RAMInKB = std::get_if<GUI::Value>(&obj.element_list[1].element);
	GUI::Input *ROMFile = std::get_if<GUI::Input>(&obj.element_list[2].element);
	GUI::Button *LoadROM = std::get_if<GUI::Button>(&obj.element_list[3].element);
	GUI::Button *ExpansionBoardOptions = std::get_if<GUI::Button>(&obj.element_list[4].element);
	GUI::Button *ReturnToMainMenu = std::get_if<GUI::Button>(&obj.element_list[6].element);
	switch (obj.current_menu_item)
	{
		case 0:
		{
			RAMInKB->select = false;
			break;
		}
		case 1:
		{
			ROMFile->select = false;
			break;
		}
		case 2:
		{
			LoadROM->select = false;
			break;
		}
		case 3:
		{
			ExpansionBoardOptions->select = false;
			break;
		}
		case 4:
		{
			ReturnToMainMenu->select = false;
			break;
		}
	}
	obj.current_menu_item = (obj.current_menu_item == 0) ? 4 : obj.current_menu_item - 1;
	bool selected = false;
	while (!selected)
	{
		switch (obj.current_menu_item)
		{
			case 0:
			{
				RAMInKB->select = true;
				selected = true;
				break;
			}
			case 1:
			{
				ROMFile->select = true;
				selected = true;
				break;
			}
			case 2:
			{
				if (!LoadROM->disabled)
				{
					LoadROM->select = true;
					selected = true;
				}
				else
				{
					--obj.current_menu_item;
				}
				break;
			}
			case 3:
			{
				ExpansionBoardOptions->select = true;
				selected = true;
				break;
			}
			case 4:
			{
				ReturnToMainMenu->select = true;
				selected = true;
				break;
			}
		}
	}
	app->DrawCurrentMenu();
}

void VIPR_Emulator::machine_options_down(VIPR_Emulator::GUI::Menu &obj, void *userdata)
{
	Application *app = static_cast<Application *>(userdata);
	GUI::Value *RAMInKB = std::get_if<GUI::Value>(&obj.element_list[1].element);
	GUI::Input *ROMFile = std::get_if<GUI::Input>(&obj.element_list[2].element);
	GUI::Button *LoadROM = std::get_if<GUI::Button>(&obj.element_list[3].element);
	GUI::Button *ExpansionBoardOptions = std::get_if<GUI::Button>(&obj.element_list[4].element);
	GUI::Button *ReturnToMainMenu = std::get_if<GUI::Button>(&obj.element_list[6].element);
	switch (obj.current_menu_item)
	{
		case 0:
		{
			RAMInKB->select = false;
			break;
		}
		case 1:
		{
			ROMFile->select = false;
			break;
		}
		case 2:
		{
			LoadROM->select = false;
			break;
		}
		case 3:
		{
			ExpansionBoardOptions->select = false;
			break;
		}
		case 4:
		{
			ReturnToMainMenu->select = false;
			break;
		}
	}
	obj.current_menu_item = (obj.current_menu_item == 4) ? 0 : obj.current_menu_item + 1;
	bool selected = false;
	while (!selected)
	{
		switch (obj.current_menu_item)
		{
			case 0:
			{
				RAMInKB->select = true;
				selected = true;
				break;
			}
			case 1:
			{
				ROMFile->select = true;
				selected = true;
				break;
			}
			case 2:
			{
				if (!LoadROM->disabled)
				{
					LoadROM->select = true;
					selected = true;
				}
				else
				{
					++obj.current_menu_item;
				}
				break;
			}
			case 3:
			{
				ExpansionBoardOptions->select = true;
				selected = true;
				break;
			}
			case 4:
			{
				ReturnToMainMenu->select = true;
				selected = true;
				break;
			}
		}
	}
	app->DrawCurrentMenu();
}

void VIPR_Emulator::machine_options_left(VIPR_Emulator::GUI::Menu &obj, void *userdata)
{
	Application *app = static_cast<Application *>(userdata);
	GUI::Value *RAMInKB = std::get_if<GUI::Value>(&obj.element_list[1].element);
	switch (obj.current_menu_item)
	{
		case 0:
		{
			if (RAMInKB->value > RAMInKB->min)
			{
				--RAMInKB->value;
				app->System.AdjustRAM(RAMInKB->value);
			}
			break;
		}
	}
	app->DrawCurrentMenu();
}

void VIPR_Emulator::machine_options_right(VIPR_Emulator::GUI::Menu &obj, void *userdata)
{
	Application *app = static_cast<Application *>(userdata);
	GUI::Value *RAMInKB = std::get_if<GUI::Value>(&obj.element_list[1].element);
	switch (obj.current_menu_item)
	{
		case 0:
		{
			if (RAMInKB->value < RAMInKB->max)
			{
				++RAMInKB->value;
				app->System.AdjustRAM(RAMInKB->value);
			}
			break;
		}
	}
	app->DrawCurrentMenu();
}

void VIPR_Emulator::machine_options_activate(VIPR_Emulator::GUI::Menu &obj, void *userdata)
{
	Application *app = static_cast<Application *>(userdata);
	GUI::Value *RAMInKB = std::get_if<GUI::Value>(&obj.element_list[1].element);
	GUI::Input *ROMFile = std::get_if<GUI::Input>(&obj.element_list[2].element);
	GUI::Status *ROMStatus = std::get_if<GUI::Status>(&obj.element_list[5].element);
	switch (obj.current_menu_item)
	{
		case 0:
		{
			app->InputFocus = &obj.element_list[1];
			RAMInKB->focus = true;
			std::ostringstream current_stream;
			if (RAMInKB->base == GUI::ValueBaseType::Hexadecimal)
			{
				current_stream << std::hex;
			}
			current_stream << RAMInKB->value;
			RAMInKB->input = current_stream.str();
			RAMInKB->cursor_pos = RAMInKB->input.size();
			app->SetOperationMode(OperationMode::Input);
			break;
		}
		case 1:
		{
			app->InputFocus = &obj.element_list[2];
			ROMFile->focus = true;
			ROMFile->input = ROMFile->stored_input;
			ROMFile->cursor_pos = ROMFile->input.size();
			if (ROMFile->input.size() >= ROMFile->max_display)
			{
				ROMFile->display_start = ROMFile->input.size() - ROMFile->max_display + 1;
			}
			app->SetOperationMode(OperationMode::Input);
			break;
		}
		case 2:
		{
			std::vector<uint8_t> ROMFileData;
			{
				std::ifstream target_rom_file(ROMFile->stored_input, std::ios::binary | std::ios::ate);
				if (target_rom_file.fail())
				{
					ROMStatus->status = "Failed";
					ROMStatus->status_color = { 0xFF, 0x00, 0x00 };
					break;
				}
				size_t rom_file_size = target_rom_file.tellg();
				if (rom_file_size > 32768)
				{
					ROMStatus->status = "ROM Too Large";
					ROMStatus->status_color = { 0xFF, 0xFF, 0x00 };
					break;
				}
				ROMFileData.resize(rom_file_size);
				target_rom_file.seekg(0, std::ios::beg);
				target_rom_file.read(reinterpret_cast<char *>(ROMFileData.data()), rom_file_size);
			}
			app->System.InstallROM(std::move(ROMFileData));
			ROMStatus->status = "Installed";
			ROMStatus->status_color = { 0x00, 0xFF, 0x00 };
			break;
		}
		case 3:
		{
			app->CurrentMenu = &app->ExpansionBoardOptionsMenu;
			break;
		}
		case 4:
		{
			app->CurrentMenu = &app->MainMenu;
			break;
		}
	}
	app->DrawCurrentMenu();
}

void VIPR_Emulator::machine_options_ram_in_kb_input_complete(VIPR_Emulator::GUI::Value &obj, void *userdata)
{
	Application *app = static_cast<Application *>(userdata);
	app->System.AdjustRAM(obj.value);
}

void VIPR_Emulator::machine_options_rom_file_input_complete(VIPR_Emulator::GUI::Input &obj, void *userdata)
{
	Application *app = static_cast<Application *>(userdata);
	GUI::Menu &MachineOptionsMenu = app->MachineOptionsMenu;
	GUI::Input *ROMFile = std::get_if<GUI::Input>(&MachineOptionsMenu.element_list[2].element);
	GUI::Button *LoadROM = std::get_if<GUI::Button>(&MachineOptionsMenu.element_list[3].element);
	GUI::Status *ROMStatus = std::get_if<GUI::Status>(&MachineOptionsMenu.element_list[5].element);
	LoadROM->disabled = (ROMFile->stored_input.size() > 0) ? false : true;
	ROMStatus->status = "None";
	ROMStatus->status_color = { 0x40, 0x40, 0x40 };
	app->DrawCurrentMenu();
}

void VIPR_Emulator::expansion_board_options_up(VIPR_Emulator::GUI::Menu &obj, void *userdata)
{
	Application *app = static_cast<Application *>(userdata);
	GUI::Toggle *VP585ExpansionKeypadInterface = std::get_if<GUI::Toggle>(&obj.element_list[1].element);
	GUI::Toggle *VP590ColorBoard = std::get_if<GUI::Toggle>(&obj.element_list[2].element);
	GUI::Toggle *VP595SimpleSoundBoard = std::get_if<GUI::Toggle>(&obj.element_list[3].element);
	GUI::Button *ReturnToMachineOptions = std::get_if<GUI::Button>(&obj.element_list[4].element);
	switch (obj.current_menu_item)
	{
		case 0:
		{
			VP585ExpansionKeypadInterface->select = false;
			break;
		}
		case 1:
		{
			VP590ColorBoard->select = false;
			break;
		}
		case 2:
		{
			VP595SimpleSoundBoard->select = false;
			break;
		}
		case 3:
		{
			ReturnToMachineOptions->select = false;
			break;
		}
	}
	obj.current_menu_item = (obj.current_menu_item == 0) ? 3 : obj.current_menu_item - 1;
	bool selected = false;
	while (!selected)
	{
		switch (obj.current_menu_item)
		{
			case 0:
			{
				VP585ExpansionKeypadInterface->select = true;
				selected = true;
				break;
			}
			case 1:
			{
				VP590ColorBoard->select = true;
				selected = true;
				break;
			}
			case 2:
			{
				VP595SimpleSoundBoard->select = true;
				selected = true;
				break;
			}
			case 3:
			{
				ReturnToMachineOptions->select = true;
				selected = true;
				break;
			}
		}
	}
	app->DrawCurrentMenu();
}

void VIPR_Emulator::expansion_board_options_down(VIPR_Emulator::GUI::Menu &obj, void *userdata)
{
	Application *app = static_cast<Application *>(userdata);
	GUI::Toggle *VP585ExpansionKeypadInterface = std::get_if<GUI::Toggle>(&obj.element_list[1].element);
	GUI::Toggle *VP590ColorBoard = std::get_if<GUI::Toggle>(&obj.element_list[2].element);
	GUI::Toggle *VP595SimpleSoundBoard = std::get_if<GUI::Toggle>(&obj.element_list[3].element);
	GUI::Button *ReturnToMachineOptions = std::get_if<GUI::Button>(&obj.element_list[4].element);
	switch (obj.current_menu_item)
	{
		case 0:
		{
			VP585ExpansionKeypadInterface->select = false;
			break;
		}
		case 1:
		{
			VP590ColorBoard->select = false;
			break;
		}
		case 2:
		{
			VP595SimpleSoundBoard->select = false;
			break;
		}
		case 3:
		{
			ReturnToMachineOptions->select = false;
			break;
		}
	}
	obj.current_menu_item = (obj.current_menu_item == 3) ? 0 : obj.current_menu_item + 1;
	bool selected = false;
	while (!selected)
	{
		switch (obj.current_menu_item)
		{
			case 0:
			{
				VP585ExpansionKeypadInterface->select = true;
				selected = true;
				break;
			}
			case 1:
			{
				VP590ColorBoard->select = true;
				selected = true;
				break;
			}
			case 2:
			{
				VP595SimpleSoundBoard->select = true;
				selected = true;
				break;
			}
			case 3:
			{
				ReturnToMachineOptions->select = true;
				selected = true;
				break;
			}
		}
	}
	app->DrawCurrentMenu();
}

void VIPR_Emulator::expansion_board_options_left(VIPR_Emulator::GUI::Menu &obj, void *userdata)
{
}

void VIPR_Emulator::expansion_board_options_right(VIPR_Emulator::GUI::Menu &obj, void *userdata)
{
}

void VIPR_Emulator::expansion_board_options_activate(VIPR_Emulator::GUI::Menu &obj, void *userdata)
{
	Application *app = static_cast<Application *>(userdata);
	GUI::Toggle *VP585ExpansionKeypadInterface = std::get_if<GUI::Toggle>(&obj.element_list[1].element);
	GUI::Toggle *VP590ColorBoard = std::get_if<GUI::Toggle>(&obj.element_list[2].element);
	GUI::Toggle *VP595SimpleSoundBoard = std::get_if<GUI::Toggle>(&obj.element_list[3].element);
	switch (obj.current_menu_item)
	{
		case 0:
		{
			VP585ExpansionKeypadInterface->toggle = (VP585ExpansionKeypadInterface->toggle) ? false : true;
			if (!VP585ExpansionKeypadInterface->toggle)
			{
				app->System.UninstallExpansionBoard(ExpansionBoardType::VP585_ExpansionKeypadInterface);
			}
			else
			{
				if (VP590ColorBoard->toggle)
				{
					VP590ColorBoard->toggle = false;
					app->System.UninstallExpansionBoard(ExpansionBoardType::VP590_ColorBoard);
				}
				app->System.InstallExpansionBoard(ExpansionBoardType::VP585_ExpansionKeypadInterface);
			}
			break;
		}
		case 1:
		{
			break;
		}
		case 2:
		{
			VP595SimpleSoundBoard->toggle = (VP595SimpleSoundBoard->toggle) ? false : true;
			if (!VP595SimpleSoundBoard->toggle)
			{
				app->System.UninstallExpansionBoard(ExpansionBoardType::VP595_SimpleSoundBoard);
			}
			else
			{
				app->System.InstallExpansionBoard(ExpansionBoardType::VP595_SimpleSoundBoard);
			}
			GUI::MultiChoice *OutputAudioDevice = std::get_if<GUI::MultiChoice>(&app->EmulatorOptionsMenu.element_list[1].element);
			app->System.SetupAudio(OutputAudioDevice->choice_list[OutputAudioDevice->current_choice]);
			break;
		}
		case 3:
		{
			app->CurrentMenu = &app->MachineOptionsMenu;
		}
	}
	app->DrawCurrentMenu();
}

void VIPR_Emulator::machine_memory_transfer_up(VIPR_Emulator::GUI::Menu &obj, void *userdata)
{
	Application *app = static_cast<Application *>(userdata);
	GUI::Input *MemoryFile = std::get_if<GUI::Input>(&obj.element_list[1].element);
	GUI::MultiChoice *TransferType = std::get_if<GUI::MultiChoice>(&obj.element_list[2].element);
	GUI::Value *StartAddress = std::get_if<GUI::Value>(&obj.element_list[3].element);
	GUI::Value *Size = std::get_if<GUI::Value>(&obj.element_list[4].element);
	GUI::Button *Transfer = std::get_if<GUI::Button>(&obj.element_list[5].element);
	GUI::Button *ReturnToMainMenu = std::get_if<GUI::Button>(&obj.element_list[7].element);
	switch (obj.current_menu_item)
	{
		case 0:
		{
			MemoryFile->select = false;
			break;
		}
		case 1:
		{
			TransferType->select = false;
			break;
		}
		case 2:
		{
			StartAddress->select = false;
			break;
		}
		case 3:
		{
			Size->select = false;
			break;
		}
		case 4:
		{
			Transfer->select = false;
			break;
		}
		case 5:
		{
			ReturnToMainMenu->select = false;
			break;
		}
	}
	obj.current_menu_item = (obj.current_menu_item == 0) ? 5 : obj.current_menu_item - 1;
	bool selected = false;
	while (!selected)
	{
		switch (obj.current_menu_item)
		{
			case 0:
			{
				MemoryFile->select = true;
				selected = true;
				break;
			}
			case 1:
			{
				TransferType->select = true;
				selected = true;
				break;
			}
			case 2:
			{
				StartAddress->select = true;
				selected = true;
				break;
			}
			case 3:
			{
				Size->select = true;
				selected = true;
				break;
			}
			case 4:
			{
				if (!Transfer->disabled)
				{
					Transfer->select = true;
					selected = true;
				}
				else
				{
					--obj.current_menu_item;
				}
				break;
			}
			case 5:
			{
				ReturnToMainMenu->select = true;
				selected = true;
				break;
			}
		}
	}
	app->DrawCurrentMenu();
}

void VIPR_Emulator::machine_memory_transfer_down(VIPR_Emulator::GUI::Menu &obj, void *userdata)
{
	Application *app = static_cast<Application *>(userdata);
	GUI::Input *MemoryFile = std::get_if<GUI::Input>(&obj.element_list[1].element);
	GUI::MultiChoice *TransferType = std::get_if<GUI::MultiChoice>(&obj.element_list[2].element);
	GUI::Value *StartAddress = std::get_if<GUI::Value>(&obj.element_list[3].element);
	GUI::Value *Size = std::get_if<GUI::Value>(&obj.element_list[4].element);
	GUI::Button *Transfer = std::get_if<GUI::Button>(&obj.element_list[5].element);
	GUI::Button *ReturnToMainMenu = std::get_if<GUI::Button>(&obj.element_list[7].element);
	switch (obj.current_menu_item)
	{
		case 0:
		{
			MemoryFile->select = false;
			break;
		}
		case 1:
		{
			TransferType->select = false;
			break;
		}
		case 2:
		{
			StartAddress->select = false;
			break;
		}
		case 3:
		{
			Size->select = false;
			break;
		}
		case 4:
		{
			Transfer->select = false;
			break;
		}
		case 5:
		{
			ReturnToMainMenu->select = false;
			break;
		}
	}
	obj.current_menu_item = (obj.current_menu_item == 5) ? 0 : obj.current_menu_item + 1;
	bool selected = false;
	while (!selected)
	{
		switch (obj.current_menu_item)
		{
			case 0:
			{
				MemoryFile->select = true;
				selected = true;
				break;
			}
			case 1:
			{
				TransferType->select = true;
				selected = true;
				break;
			}
			case 2:
			{
				StartAddress->select = true;
				selected = true;
				break;
			}
			case 3:
			{
				Size->select = true;
				selected = true;
				break;
			}
			case 4:
			{
				if (!Transfer->disabled)
				{
					Transfer->select = true;
					selected = true;
				}
				else
				{
					++obj.current_menu_item;
				}
				break;
			}
			case 5:
			{
				ReturnToMainMenu->select = true;
				selected = true;
				break;
			}
		}
	}
	app->DrawCurrentMenu();
}

void VIPR_Emulator::machine_memory_transfer_left(VIPR_Emulator::GUI::Menu &obj, void *userdata)
{
	Application *app = static_cast<Application *>(userdata);
	GUI::MultiChoice *TransferType = std::get_if<GUI::MultiChoice>(&obj.element_list[2].element);
	GUI::Status *TransferStatus = std::get_if<GUI::Status>(&obj.element_list[6].element);
	switch (obj.current_menu_item)
	{
		case 1:
		{
			TransferType->current_choice = (TransferType->current_choice == 0) ? TransferType->choice_list.size() - 1 : TransferType->current_choice - 1;
			TransferStatus->status = "None";
			TransferStatus->status_color = { 0x40, 0x40, 0x40 };
			break;
		}
	}
	app->DrawCurrentMenu();
}

void VIPR_Emulator::machine_memory_transfer_right(VIPR_Emulator::GUI::Menu &obj, void *userdata)
{
	Application *app = static_cast<Application *>(userdata);
	GUI::MultiChoice *TransferType = std::get_if<GUI::MultiChoice>(&obj.element_list[2].element);
	GUI::Status *TransferStatus = std::get_if<GUI::Status>(&obj.element_list[6].element);
	switch (obj.current_menu_item)
	{
		case 1:
		{
			TransferType->current_choice = (TransferType->current_choice == TransferType->choice_list.size() - 1) ? 0 : TransferType->current_choice + 1;
			TransferStatus->status = "None";
			TransferStatus->status_color = { 0x40, 0x40, 0x40 };
			break;
		}
	}
	app->DrawCurrentMenu();
}

void VIPR_Emulator::machine_memory_transfer_activate(VIPR_Emulator::GUI::Menu &obj, void *userdata)
{
	Application *app = static_cast<Application *>(userdata);
	GUI::Input *MemoryFile = std::get_if<GUI::Input>(&obj.element_list[1].element);
	GUI::MultiChoice *TransferType = std::get_if<GUI::MultiChoice>(&obj.element_list[2].element);
	GUI::Value *StartAddress = std::get_if<GUI::Value>(&obj.element_list[3].element);
	GUI::Value *Size = std::get_if<GUI::Value>(&obj.element_list[4].element);
	GUI::Button *Transfer = std::get_if<GUI::Button>(&obj.element_list[5].element);
	GUI::Status *TransferStatus = std::get_if<GUI::Status>(&obj.element_list[6].element);
	switch (obj.current_menu_item)
	{
		case 0:
		{
			app->InputFocus = &obj.element_list[1];
			MemoryFile->focus = true;
			MemoryFile->input = MemoryFile->stored_input;
			MemoryFile->cursor_pos = MemoryFile->input.size();
			if (MemoryFile->input.size() >= MemoryFile->max_display)
			{
				MemoryFile->display_start = MemoryFile->input.size() - MemoryFile->max_display + 1;
			}
			app->SetOperationMode(OperationMode::Input);
			break;
		}
		case 1:
		{
			TransferType->current_choice = (TransferType->current_choice == TransferType->choice_list.size() - 1) ? 0 : TransferType->current_choice + 1;
			TransferStatus->status = "None";
			TransferStatus->status_color = { 0x40, 0x40, 0x40 };
			break;
		}
		case 2:
		{
			app->InputFocus = &obj.element_list[3];
			StartAddress->focus = true;
			std::ostringstream current_stream;
			if (StartAddress->base == GUI::ValueBaseType::Hexadecimal)
			{
				current_stream << std::hex;
			}
			current_stream << StartAddress->value;
			StartAddress->input = current_stream.str();
			StartAddress->cursor_pos = StartAddress->input.size();
			app->SetOperationMode(OperationMode::Input);
			break;
		}
		case 3:
		{
			app->InputFocus = &obj.element_list[4];
			Size->focus = true;
			std::ostringstream current_stream;
			if (Size->base == GUI::ValueBaseType::Hexadecimal)
			{
				current_stream << std::hex;
			}
			current_stream << Size->value;
			Size->input = current_stream.str();
			Size->cursor_pos = Size->input.size();
			app->SetOperationMode(OperationMode::Input);
			break;
		}
		case 4:
		{
			switch (TransferType->current_choice)
			{
				case 0:
				{
					std::ifstream target_memory_file(MemoryFile->stored_input, std::ios::binary);
					if (target_memory_file.fail() || StartAddress->value + Size->value > app->System.GetRAM())
					{
						TransferStatus->status = "Failed";
						TransferStatus->status_color = { 0xFF, 0x00, 0x00 };
					}
					else
					{
						uint8_t *RAM = app->System.GetRAMData();
						target_memory_file.read(reinterpret_cast<char *>(&RAM[StartAddress->value]), Size->value);
						TransferStatus->status = "Successful";
						TransferStatus->status_color = { 0x00, 0xFF, 0x00 };
					}
					break;
				}
				case 1:
				{
					if (StartAddress->value + Size->value > app->System.GetRAM())
					{
						TransferStatus->status = "Failed";
						TransferStatus->status_color = { 0xFF, 0x00, 0x00 };
					}
					else
					{
						bool exists = false;
						{
							std::ifstream target_memory_file(MemoryFile->stored_input, std::ios::binary);
							exists = target_memory_file.is_open();
						}
						if (!exists || TransferStatus->status == "Overwrite?")
						{
							std::ofstream target_memory_file(MemoryFile->stored_input, std::ios::binary);
							uint8_t *RAM = app->System.GetRAMData();
							target_memory_file.write(reinterpret_cast<const char *>(&RAM[StartAddress->value]), Size->value);
							TransferStatus->status = "Successful";
							TransferStatus->status_color = { 0x00, 0xFF, 0x00 };
						}
						else
						{
							TransferStatus->status = "Overwrite?";
							TransferStatus->status_color = { 0xFF, 0xFF, 0x00 };
						}
					}
					break;
				}
			}
			break;
		}
		case 5:
		{
			TransferStatus->status = "None";
			TransferStatus->status_color = { 0x40, 0x40, 0x40 };
			app->CurrentMenu = &app->MainMenu;
			break;
		}
	}
	app->DrawCurrentMenu();
}

void VIPR_Emulator::machine_memory_transfer_memory_file_input_complete(VIPR_Emulator::GUI::Input &obj, void *userdata)
{
	Application *app = static_cast<Application *>(userdata);
	GUI::Menu &MachineMemoryTransferMenu = app->MachineMemoryTransferMenu;
	GUI::Input *MemoryFile = std::get_if<GUI::Input>(&MachineMemoryTransferMenu.element_list[1].element);
	GUI::Button *Transfer = std::get_if<GUI::Button>(&MachineMemoryTransferMenu.element_list[5].element);
	GUI::Status *TransferStatus = std::get_if<GUI::Status>(&MachineMemoryTransferMenu.element_list[6].element);
	Transfer->disabled = (MemoryFile->stored_input.size() > 0) ? false : true;
	TransferStatus->status = "None";
	TransferStatus->status_color = { 0x40, 0x40, 0x40 };
	app->DrawCurrentMenu();
}

void VIPR_Emulator::machine_memory_transfer_start_address_input_complete(VIPR_Emulator::GUI::Value &obj, void *userdata)
{
	Application *app = static_cast<Application *>(userdata);
	GUI::Menu &MachineMemoryTransferMenu = app->MachineMemoryTransferMenu;
	GUI::Status *TransferStatus = std::get_if<GUI::Status>(&MachineMemoryTransferMenu.element_list[6].element);
	TransferStatus->status = "None";
	TransferStatus->status_color = { 0x40, 0x40, 0x40 };
	app->DrawCurrentMenu();
}

void VIPR_Emulator::machine_memory_transfer_size_input_complete(VIPR_Emulator::GUI::Value &obj, void *userdata)
{
	Application *app = static_cast<Application *>(userdata);
	GUI::Menu &MachineMemoryTransferMenu = app->MachineMemoryTransferMenu;
	GUI::Status *TransferStatus = std::get_if<GUI::Status>(&MachineMemoryTransferMenu.element_list[6].element);
	TransferStatus->status = "None";
	TransferStatus->status_color = { 0x40, 0x40, 0x40 };
	app->DrawCurrentMenu();
}

void VIPR_Emulator::emulator_options_up(VIPR_Emulator::GUI::Menu &obj, void *userdata)
{
	Application *app = static_cast<Application *>(userdata);
	GUI::MultiChoice *OutputAudioDevice = std::get_if<GUI::MultiChoice>(&obj.element_list[1].element);
	GUI::Value *MainVolume = std::get_if<GUI::Value>(&obj.element_list[2].element);
	GUI::Button *ReturnToMainMenu = std::get_if<GUI::Button>(&obj.element_list[3].element);
	switch (obj.current_menu_item)
	{
		case 0:
		{
			OutputAudioDevice->select = false;
			break;
		}
		case 1:
		{
			MainVolume->select = false;
			break;
		}
		case 2:
		{
			ReturnToMainMenu->select = false;
			break;
		}
	}
	obj.current_menu_item = (obj.current_menu_item == 0) ? 2 : obj.current_menu_item - 1;
	bool selected = false;
	while (!selected)
	{
		switch (obj.current_menu_item)
		{
			case 0:
			{
				OutputAudioDevice->select = true;
				selected = true;
				break;
			}
			case 1:
			{
				MainVolume->select = true;
				selected = true;
				break;
			}
			case 2:
			{
				ReturnToMainMenu->select = true;
				selected = true;
				break;
			}
		}
	}
	app->DrawCurrentMenu();
}

void VIPR_Emulator::emulator_options_down(VIPR_Emulator::GUI::Menu &obj, void *userdata)
{
	Application *app = static_cast<Application *>(userdata);
	GUI::MultiChoice *OutputAudioDevice = std::get_if<GUI::MultiChoice>(&obj.element_list[1].element);
	GUI::Value *MainVolume = std::get_if<GUI::Value>(&obj.element_list[2].element);
	GUI::Button *ReturnToMainMenu = std::get_if<GUI::Button>(&obj.element_list[3].element);
	switch (obj.current_menu_item)
	{
		case 0:
		{
			OutputAudioDevice->select = false;
			break;
		}
		case 1:
		{
			MainVolume->select = false;
			break;
		}
		case 2:
		{
			ReturnToMainMenu->select = false;
			break;
		}
	}
	obj.current_menu_item = (obj.current_menu_item == 2) ? 0 : obj.current_menu_item + 1;
	bool selected = false;
	while (!selected)
	{
		switch (obj.current_menu_item)
		{
			case 0:
			{
				OutputAudioDevice->select = true;
				selected = true;
				break;
			}
			case 1:
			{
				MainVolume->select = true;
				selected = true;
				break;
			}
			case 2:
			{
				ReturnToMainMenu->select = true;
				selected = true;
				break;
			}
		}
	}
	app->DrawCurrentMenu();
}

void VIPR_Emulator::emulator_options_left(VIPR_Emulator::GUI::Menu &obj, void *userdata)
{
	Application *app = static_cast<Application *>(userdata);
	GUI::MultiChoice *OutputAudioDevice = std::get_if<GUI::MultiChoice>(&obj.element_list[1].element);
	GUI::Value *MainVolume = std::get_if<GUI::Value>(&obj.element_list[2].element);
	switch (obj.current_menu_item)
	{
		case 0:
		{
			OutputAudioDevice->current_choice = (OutputAudioDevice->current_choice == 0) ? OutputAudioDevice->choice_list.size() - 1 : OutputAudioDevice->current_choice - 1;
			app->System.SetupAudio(OutputAudioDevice->choice_list[OutputAudioDevice->current_choice]);
			break;
		}
		case 1:
		{
			if (MainVolume->value > MainVolume->min)
			{
				--MainVolume->value;
				app->System.AdjustVolume(MainVolume->value);
			}
			break;
		}
	}
	app->DrawCurrentMenu();
}

void VIPR_Emulator::emulator_options_right(VIPR_Emulator::GUI::Menu &obj, void *userdata)
{
	Application *app = static_cast<Application *>(userdata);
	GUI::MultiChoice *OutputAudioDevice = std::get_if<GUI::MultiChoice>(&obj.element_list[1].element);
	GUI::Value *MainVolume = std::get_if<GUI::Value>(&obj.element_list[2].element);
	switch (obj.current_menu_item)
	{
		case 0:
		{
			OutputAudioDevice->current_choice = (OutputAudioDevice->current_choice == OutputAudioDevice->choice_list.size() - 1) ? 0 : OutputAudioDevice->current_choice + 1;
			app->System.SetupAudio(OutputAudioDevice->choice_list[OutputAudioDevice->current_choice]);
			break;
		}
		case 1:
		{
			if (MainVolume->value < MainVolume->max)
			{
				++MainVolume->value;
				app->System.AdjustVolume(MainVolume->value);
			}
			break;
		}
	}
	app->DrawCurrentMenu();
}

void VIPR_Emulator::emulator_options_activate(VIPR_Emulator::GUI::Menu &obj, void *userdata)
{
	Application *app = static_cast<Application *>(userdata);
	GUI::MultiChoice *OutputAudioDevice = std::get_if<GUI::MultiChoice>(&obj.element_list[1].element);
	switch (obj.current_menu_item)
	{
		case 0:
		{
			OutputAudioDevice->current_choice = (OutputAudioDevice->current_choice == OutputAudioDevice->choice_list.size() - 1) ? 0 : OutputAudioDevice->current_choice + 1;
			app->System.SetupAudio(OutputAudioDevice->choice_list[OutputAudioDevice->current_choice]);
			break;
		}
		case 2:
		{
			app->CurrentMenu = &app->MainMenu;
			break;
		}
	}
	app->DrawCurrentMenu();
}

int main(int argc, char *argv[])
{
	VIPR_Emulator::Application MainApp;
	if (!MainApp.Fail())
	{
		MainApp.RunMainLoop();
	}
	return MainApp.GetReturnCode();
}
