#pragma once

// -- Application Includes ---------//

// -- Graphics Includes --------------------- //
#include "GamepadCodes.h"
#include "MouseButtonCodes.h"
#include "KeyCodes.h"


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

	// Utility methods ------------------------------------------------- //
	Device                              DeviceFromPart(DevicePart part);

	cstring*							GamepadAxisNames();
	cstring*							GamepadButtonNames();
	cstring*							MouseButtonNames();


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
		using EventCallbackFn = std::function<void(Event&)>;

		f32                             axis[GAMEPAD_AXIS_COUNT];
		u8                              buttons[GAMEPAD_BUTTON_COUNT];
		u8                              previous_buttons[GAMEPAD_BUTTON_COUNT];

		EventCallbackFn					EventCallback;

		void*							handle;
		cstring                         name;

		u32                             index;
		i32                             id;

		bool                            IsAttached() const { return id >= 0; }
		bool                            IsButtonDown(GamepadButtons button) { return buttons[button]; }
		bool                            IsButtonJustPressed(GamepadButtons button) { return (buttons[button] && !previous_buttons[button]); }
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

		InputBinding& Set(BindingType type, Device device, DevicePart device_part, u16 button, u8 is_composite, u8 is_part_of_composite, u8 repeat);
		InputBinding& SetDeadZones(f32 min, f32 max);
		InputBinding& SetHandles(InputHandle action_map, InputHandle action);

	}; // struct InputBinding


	struct InputAction 
	{

		bool                            Triggered() const;
		f32                             ReadValue1D() const;
		InputVector2                    ReadValue2D() const;

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

		bool                            IsKeyDown(Keys key);
		bool                            IsKeyJustPressed(Keys key, bool repeat = false);
		bool                            IsKeyJustReleased(Keys key);

		// -- Mouse ------------------------------------------------------------------ //

		bool                            IsMouseDown(MouseButtons button);
		bool                            IsMouseClicked(MouseButtons button);
		bool                            IsMouseReleased(MouseButtons button);
		bool                            IsMouseDragging(MouseButtons button);


		void                            Update(f32 delta);

		void                            DebugUI();

		void                            NewFrame();            // Called before message handling
		void                            OnEvent(Event& e);

		bool                            IsTriggered(InputHandle action) const;
		f32                             IsReadValue1D(InputHandle action) const;
		InputVector2                    IsReadValue2D(InputHandle action) const;

		// Create methods used to create the actual input
		InputHandle                     CreateActionMap(const InputActionMapCreation& creation);
		InputHandle                     CreateAction(const InputActionCreation& creation);

		// Find methods using name
		InputHandle                     FindActionMap(cstring name) const;
		InputHandle                     FindAction(cstring name) const;

		void                            AddButton(InputHandle action, DevicePart device, uint16_t button, bool repeat = false);
		void                            AddAxis1D(InputHandle action, DevicePart device, uint16_t axis, float min_deadzone, float max_deadzone);
		void                            AddAxis2D(InputHandle action, DevicePart device, uint16_t x_axis, uint16_t y_axis, float min_deadzone, float max_deadzone);
		void                            AddVector1D(InputHandle action, DevicePart device_pos, uint16_t button_pos, DevicePart device_neg, uint16_t button_neg, bool repeat = true);
		void                            AddVector2D(InputHandle action, DevicePart device_up, uint16_t button_up, DevicePart device_down, uint16_t button_down, DevicePart device_left, uint16_t button_left, DevicePart device_right, uint16_t button_right, bool repeat = true);

		void							SetEventCallback(const EventCallbackFn& callback);

	public:

		Magnefu::StringBuffer            string_buffer;

		Array<InputActionMap>           action_maps;
		Array<InputAction>              actions;
		Array<InputBinding>             bindings;

		Gamepad                         gamepads[k_max_gamepads];

		u8                              keys[MF_KEY_COUNT];
		u8                              previous_keys[MF_KEY_COUNT];

		InputVector2                    mouse_position;
		InputVector2                    previous_mouse_position;
		InputVector2                    mouse_clicked_position[MOUSE_BUTTONS_COUNT];
		u8                              mouse_buttons[MOUSE_BUTTONS_COUNT];
		u8                              previous_mouse_buttons[MOUSE_BUTTONS_COUNT];
		f32                             mouse_drag_distance[MOUSE_BUTTONS_COUNT];

		bool                            has_focus;

		static constexpr cstring        k_name = "Magnefu_Input_Service";

	}; // struct InputService

}