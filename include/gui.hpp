#ifndef _GUI_HPP_
#define _GUI_HPP_

#include "renderer.hpp"
#include <cstdint>
#include <string>
#include <vector>
#include <sstream>
#include <variant>
#include <concepts>

namespace VIPR_Emulator
{
	namespace GUI
	{
		template <typename T>
		using GUIEventCallback = void (*)(T &obj, void *userdata);

		enum class ElementType
		{
			Text, Status, Button, Toggle, Input, MultiChoice, Value
		};

		enum class ValueBaseType
		{
			Decimal, Hexadecimal
		};

		struct alignas(4) ColorData
		{
			uint8_t red;
			uint8_t green;
			uint8_t blue;
		};

		template <typename T>
		concept HasText = requires(T obj)
		{
			{ obj.text } -> std::convertible_to<std::string>;
		};

		template <typename T>
		concept HasStatus = requires(T obj)
		{
			{ obj.status } -> std::convertible_to<std::string>;
		};

		template <typename T>
		concept HasStoredInput = requires(T obj)
		{
			{ obj.stored_input } -> std::convertible_to<std::string>;
		};

		template <typename T>
		concept HasInput = requires(T obj)
		{
			{ obj.input } -> std::convertible_to<std::string>;
		};

		template <typename T>
		concept HasXY = requires(T obj)
		{
			{ obj.x } -> std::convertible_to<uint16_t>;
			{ obj.y } -> std::convertible_to<uint16_t>;
		};

		template <typename T>
		concept HasColor = requires(T obj)
		{
			{ obj.color } -> std::convertible_to<ColorData>;
		};

		template <typename T>
		concept HasStatusColor = requires(T obj)
		{
			{ obj.status_color } -> std::convertible_to<ColorData>;
		};

		template <typename T>
		concept HasSelectColor = requires(T obj)
		{
			{ obj.select_color } -> std::convertible_to<ColorData>;
		};

		template <typename T>
		concept HasToggleColor = requires(T obj)
		{
			{ obj.toggle_color } -> std::convertible_to<ColorData>;
		};

		template <typename T>
		concept HasInputColor = requires(T obj)
		{
			{ obj.input_color } -> std::convertible_to<ColorData>;
		};

		template <typename T>
		concept HasChoiceColor = requires(T obj)
		{
			{ obj.choice_color } -> std::convertible_to<ColorData>;
		};

		template <typename T>
		concept HasValueColor = requires(T obj)
		{
			{ obj.value_color } -> std::convertible_to<ColorData>;
		};

		template <typename T>
		concept HasDisabledColor = requires(T obj)
		{
			{ obj.disabled_color } -> std::convertible_to<ColorData>;
		};

		template <typename T>
		concept HasSelect = requires(T obj)
		{
			{ obj.select } -> std::convertible_to<bool>;
		};

		template <typename T>
		concept HasToggle = requires(T obj)
		{
			{ obj.toggle } -> std::convertible_to<bool>;
		};

		template <typename T>
		concept HasFocus = requires(T obj)
		{
			{ obj.focus } -> std::convertible_to<bool>;
		};

		template <typename T>
		concept HasCursorPos = requires(T obj)
		{
			{ obj.cursor_pos } -> std::convertible_to<uint16_t>;
		};

		template <typename T>
		concept HasMaxDisplay = requires(T obj)
		{
			{ obj.max_display } -> std::convertible_to<uint16_t>;
		};

		template <typename T>
		concept HasDisplayStart = requires(T obj)
		{
			{ obj.display_start } -> std::convertible_to<uint16_t>;
		};

		template <typename T>
		concept HasCurrentChoice = requires(T obj)
		{
			{ obj.current_choice } -> std::convertible_to<size_t>;
		};

		template <typename T>
		concept HasChoiceList = requires(T obj)
		{
			obj.choice_list;
		};

		template <typename T>
		concept HasBase = requires(T obj)
		{
			{ obj.base } -> std::convertible_to<ValueBaseType>;
		};

		template <typename T>
		concept HasValue = requires(T obj)
		{
			{ obj.value } -> std::convertible_to<int32_t>;
		};

		template <typename T>
		concept HasMinimum = requires(T obj)
		{
			{ obj.min } -> std::convertible_to<int32_t>;
		};

		template <typename T>
		concept HasMaximum = requires(T obj)
		{
			{ obj.max } -> std::convertible_to<int32_t>;
		};

		template <typename T>
		concept HasElementList = requires(T obj)
		{
			obj.element_list;
		};

		template <typename T>
		concept HasDisabled = requires(T obj)
		{
			{ obj.disabled } -> std::convertible_to<bool>;
		};

		template <typename T>
		concept HasHidden = requires(T obj)
		{
			{ obj.hidden } -> std::convertible_to<bool>;
		};

		struct Text
		{
			std::string text;
			uint16_t x;
			uint16_t y;
			ColorData color;
			bool hidden;
		};
		
		struct Status
		{
			std::string text;
			std::string status;
			uint16_t x;
			uint16_t y;
			ColorData color, status_color;
			bool hidden;
		};

		struct Button
		{
			std::string text;
			uint16_t x;
			uint16_t y;
			ColorData color, select_color, disabled_color;
			bool select;
			bool disabled;
			bool hidden;
			GUIEventCallback<Button> on_click;
		};

		struct Toggle
		{
			std::string text;
			uint16_t x;
			uint16_t y;
			ColorData color, select_color, toggle_color;
			bool select;
			bool toggle;
			bool hidden;
			GUIEventCallback<Toggle> on_toggle;
		};

		struct Input
		{
			std::string text;
			std::string stored_input;
			std::string input;
			uint16_t x;
			uint16_t y;
			ColorData color, select_color, input_color;
			uint16_t cursor_pos;
			uint16_t max_display;
			uint16_t display_start;
			uint16_t max_length;
			bool select;
			bool focus;
			bool hidden;
			GUIEventCallback<Input> on_input_complete;
		};

		struct MultiChoice
		{
			std::string text;
			uint16_t x;
			uint16_t y;
			ColorData color, select_color, choice_color;
			size_t current_choice;
			std::vector<std::string> choice_list;
			bool select;
			bool hidden;
		};

		struct Value
		{
			std::string text;
			std::string input;
			uint16_t x;
			uint16_t y;
			ColorData color, select_color, value_color;
			ValueBaseType base;
			int32_t value;
			int32_t min;
			int32_t max;
			uint16_t cursor_pos;
			bool select;
			bool focus;
			bool hidden;
			GUIEventCallback<Value> on_input_complete;
		};

		struct ElementData
		{
			ElementType type;
			std::variant<Text, Status, Button, Toggle, Input, MultiChoice, Value> element;
		};

		struct Menu
		{
			uint16_t x;
			uint16_t y;
			std::vector<ElementData> element_list;
			size_t current_menu_item;
			bool hidden;
			GUIEventCallback<Menu> on_up;
			GUIEventCallback<Menu> on_down;
			GUIEventCallback<Menu> on_left;
			GUIEventCallback<Menu> on_right;
			GUIEventCallback<Menu> on_activate;
		};

		template <typename T> requires HasText<T> && HasXY<T> && HasColor<T> && HasHidden<T>
		void DrawElement(Renderer &renderer, T &element)
		{
			renderer.SetFontColor(element.color.red, element.color.green, element.color.blue);
			if (!element.hidden)
			{
				renderer.DrawText(element.text, element.x, element.y);
			}
		}

		template <typename T> requires HasText<T> && HasStatus<T> && HasXY<T> && HasColor<T> && HasStatusColor<T> && HasHidden<T>
		void DrawElement(Renderer &renderer, T &element)
		{
			renderer.SetFontColor(element.color.red, element.color.green, element.color.blue);
			if (!element.hidden)
			{
				std::ostringstream current_stream;
				current_stream << element.text << ':';
				std::string stream_str = current_stream.str();
				renderer.DrawText(stream_str, element.x, element.y);
				uint16_t current_x = element.x + ((stream_str.size() + 1) * 8);
				renderer.SetFontColor(element.status_color.red, element.status_color.green, element.status_color.blue);
				renderer.DrawText(element.status, current_x, element.y);
			}
		}

		template <typename T> requires HasText<T> && HasXY<T> && HasColor<T> && HasSelectColor<T> && HasDisabledColor<T> && HasSelect<T> && HasDisabled<T> && HasHidden<T>
		void DrawElement(Renderer &renderer, T &element)
		{
			ColorData &current_color = !element.select ? ((!element.disabled) ? element.color : element.disabled_color ) : element.select_color;
			renderer.SetFontColor(current_color.red, current_color.green, current_color.blue);
			if (!element.hidden)
			{
				renderer.DrawText(element.text, element.x, element.y);
			}
		}

		template <typename T> requires HasText<T> && HasXY<T> && HasColor<T> && HasSelectColor<T> && HasToggleColor<T> && HasSelect<T> && HasToggle<T> && HasHidden<T>
		void DrawElement(Renderer &renderer, T &element)
		{
			ColorData &current_color = !element.select ? element.color : element.select_color;
			renderer.SetFontColor(current_color.red, current_color.green, current_color.blue);
			if (!element.hidden)
			{
				std::ostringstream current_stream;
				current_stream << element.text << ':';
				std::string stream_str = current_stream.str();
				renderer.DrawText(stream_str, element.x, element.y);
				uint16_t current_x = element.x + ((stream_str.size() + 1) * 8);
				renderer.SetFontColor(element.toggle_color.red, element.toggle_color.green, element.toggle_color.blue);
				renderer.DrawText(element.toggle ? "On" : "Off", current_x, element.y);
			}
		}

		template <typename T> requires HasText<T> && HasStoredInput<T> && HasInput<T> && HasXY<T> && HasColor<T> && HasSelectColor<T> && HasInputColor<T> && HasSelect<T> && HasFocus<T> && HasCursorPos<T> && HasMaxDisplay<T> && HasDisplayStart<T> && HasHidden<T>
		void DrawElement(Renderer &renderer, T &element)
		{
			ColorData &current_color = !element.select ? element.color : element.select_color;
			renderer.SetFontColor(current_color.red, current_color.green, current_color.blue);
			if (!element.hidden)
			{
				std::ostringstream current_stream;
				current_stream << element.text << ':';
				std::string stream_str = current_stream.str();
				renderer.DrawText(stream_str, element.x, element.y);
				uint16_t current_x = element.x + ((stream_str.size() + 1) * 8);
				renderer.SetFontColor(element.input_color.red, element.input_color.green, element.input_color.blue);
				uint16_t input_count = element.focus ? element.input.size() : element.stored_input.size();
				for (uint16_t i = element.display_start; i < input_count + 1 && i < element.display_start + element.max_display; ++i)
				{
					if (i == element.cursor_pos && element.focus)
					{
						renderer.SetFontFlags(0x01);
					}
					renderer.DrawChar(element.focus ? (i < input_count ? element.input[i] : 32) : (i < input_count ? element.stored_input[i] : 32), current_x, element.y);
					renderer.SetFontFlags(0x00);
					current_x += 8;
				}
			}
		}	

		template <typename T> requires HasText<T> && HasXY<T> && HasColor<T> && HasSelectColor<T> && HasChoiceColor<T> && HasCurrentChoice<T> && HasChoiceList<T> && HasHidden<T>
		void DrawElement(Renderer &renderer, T &element)
		{
			ColorData &current_color = !element.select ? element.color : element.select_color;
			renderer.SetFontColor(current_color.red, current_color.green, current_color.blue);
			if (!element.hidden)
			{
				std::ostringstream current_stream;
				current_stream << element.text << ':';
				std::string stream_str = current_stream.str();
				renderer.DrawText(stream_str, element.x, element.y);
				if (element.current_choice < element.choice_list.size())
				{
					uint16_t current_x = element.x + ((stream_str.size() + 1) * 8);
					renderer.SetFontColor(element.choice_color.red, element.choice_color.green, element.choice_color.blue);
					renderer.DrawText(element.choice_list[element.current_choice], current_x, element.y);
				};
			}
		}

		template <typename T> requires HasText<T> && HasInput<T> && HasXY<T> && HasColor<T> && HasSelectColor<T> && HasValueColor<T> && HasSelect<T> && HasBase<T> && HasValue<T> && HasCursorPos<T> && HasFocus<T> && HasHidden<T>
		void DrawElement(Renderer &renderer, T &element)
		{
			ColorData &current_color = !element.select ? element.color : element.select_color;
			renderer.SetFontColor(current_color.red, current_color.green, current_color.blue);
			if (!element.hidden)
			{
				std::ostringstream current_stream;
				current_stream << element.text << ':';
				std::string stream_str = current_stream.str();
				renderer.DrawText(stream_str, element.x, element.y);
				uint16_t current_x = element.x + ((stream_str.size() + 1) * 8);
				current_stream.str("");
				renderer.SetFontColor(element.value_color.red, element.value_color.green, element.value_color.blue);
				if (element.base == ValueBaseType::Hexadecimal)
				{
					current_stream << "0x";
					current_stream << std::hex;
				}
				if (!element.focus)
				{
					current_stream << element.value;
					renderer.DrawText(current_stream.str(), current_x, element.y);
				}
				else
				{
					if (element.base == ValueBaseType::Hexadecimal)
					{
						renderer.DrawText(current_stream.str(), current_x, element.y);
						current_x += 8 * current_stream.str().size();
					}
					uint16_t input_count = element.input.size();
					for (uint16_t i = 0; i < input_count + 1; ++i)
					{
						if (i == element.cursor_pos)
						{
							renderer.SetFontFlags(0x01);
						}
						renderer.DrawChar(i < input_count ? element.input[i] : 32, current_x, element.y);
						renderer.SetFontFlags(0x00);
						current_x += 8;
					}
				}
			}
		}

		template <typename T> requires HasXY<T> && HasElementList<T> && HasHidden<T>
		void DrawElement(Renderer &renderer, T &element)
		{
			if (!element.hidden)
			{
				for (auto &i : element.element_list)
				{
					switch (i.type)
					{
						case ElementType::Text:
						{
							Text current_element = std::get<Text>(i.element);
							current_element.x += element.x;
							current_element.y += element.y;
							DrawElement(renderer, current_element);
							break;
						}
						case ElementType::Status:
						{
							Status current_element = std::get<Status>(i.element);
							current_element.x += element.x;
							current_element.y += element.y;
							DrawElement(renderer, current_element);
							break;
						}
						case ElementType::Button:
						{
							Button current_element = std::get<Button>(i.element);
							current_element.x += element.x;
							current_element.y += element.y;
							DrawElement(renderer, current_element);
							break;
						}
						case ElementType::Toggle:
						{
							Toggle current_element = std::get<Toggle>(i.element);
							current_element.x += element.x;
							current_element.y += element.y;
							DrawElement(renderer, current_element);
							break;
						}
						case ElementType::Input:
						{
							Input current_element = std::get<Input>(i.element);
							current_element.x += element.x;
							current_element.y += element.y;
							DrawElement(renderer, current_element);
							break;
						}
						case ElementType::MultiChoice:
						{
							MultiChoice current_element = std::get<MultiChoice>(i.element);
							current_element.x += element.x;
							current_element.y += element.y;
							DrawElement(renderer, current_element);
							break;
						}
						case ElementType::Value:
						{
							Value current_element = std::get<Value>(i.element);
							current_element.x += element.x;
							current_element.y += element.y;
							DrawElement(renderer, current_element);
							break;
						}
					}
				}
			}
		}
	}
}

#endif
