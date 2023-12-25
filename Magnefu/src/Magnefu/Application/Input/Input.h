#pragma once

// -- Application Includes ---------//

// -- Graphics Includes --------------------- //
#include "GamepadCodes.h"


// -- Core Includes ---------------------------------- //
#include "Magnefu/Core/Assertions.h"
#include "Magnefu/Core/Platform.hpp"
#include "Magnefu/Core/MemoryAllocation/Memory.hpp"


namespace Magnefu
{
	enum Device : u8 
	{
		DEVICE_KEYBOARD,
		DEVICE_MOUSE,
		DEVICE_GAMEPAD
	}; // enum Device

	enum DevicePart : u8 
	{
		DEVICE_PART_KEYBOARD,
		DEVICE_PART_MOUSE,
		DEVICE_PART_GAMEPAD_AXIS,
		DEVICE_PART_GAMEPAD_BUTTONS
	}; // enum DevicePart

	enum BindingType : u8 
	{
		BINDING_TYPE_BUTTON,
		BINDING_TYPE_AXIS_1D,
		BINDING_TYPE_AXIS_2D,
		BINDING_TYPE_VECTOR_1D,
		BINDING_TYPE_VECTOR_2D,
		BINDING_TYPE_BUTTON_ONE_MOD,
		BINDING_TYPE_BUTTON_TWO_MOD
	}; // enum Type

	// Utility methods ////////////////////////////////////////////////////////
	Device                              device_from_part(DevicePart part);

	cstring* gamepad_axis_names();
	cstring* gamepad_button_names();
	cstring* mouse_button_names();


	//
	//
	struct InputVector2 
	{
		f32                             x;
		f32                             y;
	}; // struct InputVector2

	//
	//
	struct Gamepad 
	{
		f32                             axis[GAMEPAD_AXIS_COUNT];
		u8                              buttons[GAMEPAD_BUTTON_COUNT];
		u8                              previous_buttons[GAMEPAD_BUTTON_COUNT];

		void*							handle;
		cstring                         name;

		u32                             index;
		i32                             id;

		bool                            is_attached() const { return id >= 0; }
		bool                            is_button_down(GamepadButtons button) { return buttons[button]; }
		bool                            is_button_just_pressed(GamepadButtons button) { return (buttons[button] && !previous_buttons[button]); }
	}; // struct Gamepad

	//
	//
	typedef u32 InputHandle;

	typedef u32 InputHandle;

	struct InputBinding 
	{

		// Enums are u8
		BindingType                     type;
		Device                          device;
		DevicePart                      device_part;
		u8                              action_map_index;

		u16                             action_index;
		u16                             button;         // Stores the buttons either from GAMEPAD_BUTTONS_*, KEY_*, MOUSE_BUTTON_*.

		f32                             value = 0.0f;

		u8                              is_composite;
		u8                              is_part_of_composite;
		u8                              repeat;

		f32                             min_deadzone = 0.10f;
		f32                             max_deadzone = 0.95f;

		InputBinding& set(BindingType type, Device device, DevicePart device_part, u16 button, u8 is_composite, u8 is_part_of_composite, u8 repeat);
		InputBinding& set_deadzones(f32 min, f32 max);
		InputBinding& set_handles(InputHandle action_map, InputHandle action);

	}; // struct InputBinding


	struct InputAction 
	{

		bool                            triggered() const;
		f32                             read_value_1d() const;
		InputVector2                    read_value_2d() const;

		InputVector2                    value;
		InputHandle                     action_map;
		cstring                         name;

	}; // struct InputAction


	struct InputActionMap {

		cstring                         name;
		bool                            active;

	}; // struct InputActionMap

	//
	//
	struct InputActionMapCreation {
		cstring                         name;
		bool                            active;
	}; // struct InputActionMapCreation

	//
	//
	struct InputActionCreation {

		cstring                         name;
		InputHandle                     action_map;

	}; // struct InputActionCreation

	//
	//
	struct InputBindingCreation {

		InputHandle                     action;

	}; // struct InputBindingCreation


	struct InputService : public Service 
	{

		MF_DECLARE_SERVICE(InputService);

		void                            Init(Allocator* allocator);
		void                            Shutdown();

		// -- Keys ------------------------------------------------------------------ //

		bool                            is_key_down(Keys key);
		bool                            is_key_just_pressed(Keys key, bool repeat = false);
		bool                            is_key_just_released(Keys key);

		static bool IsKeyPressed(int keycode) { return s_Instance->IsKeyPressedImpl(keycode); }	
		static bool IsKeyReleased(int keycode) { return s_Instance->IsKeyReleasedImpl(keycode); }
		//static bool IsKeyDown(int keycode) { return s_Instance->IsKeyDownImpl(keycode, bool repeat = false); }

		// -- Mouse ------------------------------------------------------------------ //

		bool                            is_mouse_down(MouseButtons button);
		bool                            is_mouse_clicked(MouseButtons button);
		bool                            is_mouse_released(MouseButtons button);
		bool                            is_mouse_dragging(MouseButtons button);

		static bool IsMouseButtonPressed(int keycode) { return s_Instance->IsMouseButtonPressedImpl(keycode); }
		static bool IsMouseButtonReleased(int keycode) { return s_Instance->IsMouseButtonReleasedImpl(keycode); }

		void                            update(f32 delta);

		void                            debug_ui();

		void                            new_frame();            // Called before message handling
		void                            on_event(void* input_event);

		bool                            is_triggered(InputHandle action) const;
		f32                             is_read_value_1d(InputHandle action) const;
		InputVector2                    is_read_value_2d(InputHandle action) const;

		// Create methods used to create the actual input
		InputHandle                     create_action_map(const InputActionMapCreation& creation);
		InputHandle                     create_action(const InputActionCreation& creation);

		// Find methods using name
		InputHandle                     find_action_map(cstring name) const;
		InputHandle                     find_action(cstring name) const;

		void                            add_button(InputHandle action, DevicePart device, uint16_t button, bool repeat = false);
		void                            add_axis_1d(InputHandle action, DevicePart device, uint16_t axis, float min_deadzone, float max_deadzone);
		void                            add_axis_2d(InputHandle action, DevicePart device, uint16_t x_axis, uint16_t y_axis, float min_deadzone, float max_deadzone);
		void                            add_vector_1d(InputHandle action, DevicePart device_pos, uint16_t button_pos, DevicePart device_neg, uint16_t button_neg, bool repeat = true);
		void                            add_vector_2d(InputHandle action, DevicePart device_up, uint16_t button_up, DevicePart device_down, uint16_t button_down, DevicePart device_left, uint16_t button_left, DevicePart device_right, uint16_t button_right, bool repeat = true);

	protected:
		virtual bool IsKeyPressedImpl(int keycode) = 0;
		virtual bool IsKeyReleasedImpl(int keycode) = 0;
		virtual bool IsMouseButtonPressedImpl(int buttoncode) = 0;
		virtual bool IsMouseButtonReleasedImpl(int buttoncode) = 0;
		virtual std::pair<float, float> GetMousePositionImpl() = 0;
		virtual float GetMouseXImpl() = 0;
		virtual float GetMouseYImpl() = 0;

	public:

		Magnefu::StringBuffer            string_buffer;

		Array<InputActionMap>           action_maps;
		Array<InputAction>              actions;
		Array<InputBinding>             bindings;

		Gamepad                         gamepads[k_max_gamepads];

		u8                              keys[KEY_COUNT];
		u8                              previous_keys[KEY_COUNT];

		InputVector2                    mouse_position;
		InputVector2                    previous_mouse_position;
		InputVector2                    mouse_clicked_position[MOUSE_BUTTONS_COUNT];
		u8                              mouse_button[MOUSE_BUTTONS_COUNT];
		u8                              previous_mouse_button[MOUSE_BUTTONS_COUNT];
		f32                             mouse_drag_distance[MOUSE_BUTTONS_COUNT];

		bool                            has_focus;

		static constexpr cstring        k_name = "Magnefu_Input_Service";
		static InputService* s_Instance;

	}; // struct InputService



	/*class  Input
	{
	public:
		
		
		static std::pair<float, float> GetMousePostion() { return s_Instance->GetMousePositionImpl(); }
		static float GetMouseX() { return s_Instance->GetMouseXImpl(); }
		static float GetMouseY() { return s_Instance->GetMouseYImpl(); }

	protected:
		virtual bool IsKeyPressedImpl(int keycode) = 0;
		virtual bool IsKeyReleasedImpl(int keycode) = 0;
		virtual bool IsMouseButtonPressedImpl(int buttoncode) = 0;
		virtual bool IsMouseButtonReleasedImpl(int buttoncode) = 0;
		virtual std::pair<float, float> GetMousePositionImpl() = 0;
		virtual float GetMouseXImpl() = 0;
		virtual float GetMouseYImpl() = 0;

	private:
		static Input* s_Instance;
	};*/
}