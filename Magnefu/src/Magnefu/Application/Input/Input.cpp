// -- PCH -- //
#include "mfpch.h"

// -- HEADER -- //
#include "Input.h"

#include "Magnefu/Application/Application.h"
#include "Magnefu/Application/Windows/WindowsWindow.h"

#include "Magnefu//Application/Events/Event.h"
#include "Magnefu/Application/Events/GamepadEvent.h"
#include "Magnefu/Application/Input/KeyCodes.h"
#include "Magnefu/Application/Input/GamepadCodes.h"


#define INPUT_BACKEND_GLFW

#if defined (INPUT_BACKEND_GLFW)

    #define GLFW_INCLUDE_VULKAN
    #include <GLFW/glfw3.h>


#endif // INPUT_BACKEND_GLFW

#include "imgui/imgui.h"


#include <cmath>

namespace Magnefu
{

    struct InputBackend 
    {

        void            Init(Gamepad* gamepads, u32 num_gamepads);
        void            Shutdown();

        void            GetMouseState(InputVector2& position, u8* buttons, u32 num_buttons);

        void            OnEvent(Event& event_, u8* keys, u32 num_keys, Gamepad* gamepads, u32 num_gamepads, bool& has_focus);

    }; // struct InputBackendSDL


#if defined (INPUT_BACKEND_GLFW)

    static void GLFWJoystickCallback(int joy, int action) 
    {
        Gamepad& pad = *(Gamepad*)glfwGetJoystickUserPointer(joy);

        if (action == GLFW_CONNECTED) 
        {
            GamepadConnectedEvent event(joy);
            pad.EventCallback(event);
        }
        else if (action == GLFW_DISCONNECTED) 
        {
            GamepadDisconnectedEvent event(joy);
            pad.EventCallback(event);
        }
    }

    static bool InitGamepad(int32_t index, Gamepad& gamepad) 
    {

        memset(&gamepad, 0, sizeof(Gamepad));
        gamepad.name = glfwGetJoystickName(index);
        GLFWgamepadstate state;

        if (glfwGetGamepadState(index, &state))
        {
            glfwSetJoystickUserPointer(index, &gamepad);
            

            gamepad.index = index;
            gamepad.id = index;
            //gamepad.handle = &gamepad;
            return true;
        }
           

        MF_CORE_ERROR("Couldn't open Joystick %u", index);
        gamepad.index = u32_max;

        return false;
    }


    static void TerminateGamepad(Gamepad& gamepad) 
    {

        gamepad.index = u32_max;
        gamepad.name = 0;
        gamepad.handle = 0;
        gamepad.id = u32_max;
    }

    // InputBackendSDL ----------------------------------------------- //
    void InputBackend::Init(Gamepad* gamepads, u32 num_gamepads) 
    {
        glfwSetJoystickCallback(GLFWJoystickCallback);

        for (u32 i = 0; i < num_gamepads; i++) {
            gamepads[i].index = u32_max;
            gamepads[i].id = u32_max;
        }

        // Iterate through the GLFW joystick IDs
        for (u32 i = 0; i < num_gamepads; i++) 
        {
            if (glfwJoystickPresent(i)) 
            {
                if (glfwJoystickIsGamepad(i)) 
                {
                    InitGamepad(i, gamepads[i]);
                }
            }
        }
    }

    void InputBackend::Shutdown() 
    {

        
    }

    



    static i32 ToMFKeyCode(i32 key) 
    {
        switch (key) 
        {
        case GLFW_KEY_UNKNOWN:        return MF_KEY_UNKNOWN;
        case GLFW_KEY_SPACE:          return MF_KEY_SPACE;
        case GLFW_KEY_APOSTROPHE:     return MF_KEY_APOSTROPHE;
        case GLFW_KEY_COMMA:          return MF_KEY_COMMA;
        case GLFW_KEY_MINUS:          return MF_KEY_MINUS;
        case GLFW_KEY_PERIOD:         return MF_KEY_PERIOD;
        case GLFW_KEY_SLASH:          return MF_KEY_SLASH;
        case GLFW_KEY_0:              return MF_KEY_0;
        case GLFW_KEY_1:              return MF_KEY_1;
        case GLFW_KEY_2:              return MF_KEY_2;
        case GLFW_KEY_3:              return MF_KEY_3;
        case GLFW_KEY_4:              return MF_KEY_4;
        case GLFW_KEY_5:              return MF_KEY_5;
        case GLFW_KEY_6:              return MF_KEY_6;
        case GLFW_KEY_7:              return MF_KEY_7;
        case GLFW_KEY_8:              return MF_KEY_8;
        case GLFW_KEY_9:              return MF_KEY_9;
            

            // Alphabetical keys
        case GLFW_KEY_A:                return MF_KEY_A;
        case GLFW_KEY_B:                return MF_KEY_B;
        case GLFW_KEY_C:                return MF_KEY_C;
        case GLFW_KEY_D:                return MF_KEY_D;
        case GLFW_KEY_E:                return MF_KEY_E;
        case GLFW_KEY_F:                return MF_KEY_F;
        case GLFW_KEY_G:                return MF_KEY_G;
        case GLFW_KEY_H:                return MF_KEY_H;
        case GLFW_KEY_I:                return MF_KEY_I;
        case GLFW_KEY_J:                return MF_KEY_J;
        case GLFW_KEY_K:                return MF_KEY_K;
        case GLFW_KEY_L:                return MF_KEY_L;
        case GLFW_KEY_M:                return MF_KEY_M;
        case GLFW_KEY_N:                return MF_KEY_N;
        case GLFW_KEY_O:                return MF_KEY_O;
        case GLFW_KEY_P:                return MF_KEY_P;
        case GLFW_KEY_Q:                return MF_KEY_Q;
        case GLFW_KEY_R:                return MF_KEY_R;
        case GLFW_KEY_S:                return MF_KEY_S;
        case GLFW_KEY_T:                return MF_KEY_T;
        case GLFW_KEY_U:                return MF_KEY_U;
        case GLFW_KEY_V:                return MF_KEY_V;
        case GLFW_KEY_W:                return MF_KEY_W;
        case GLFW_KEY_X:                return MF_KEY_X;
        case GLFW_KEY_Y:                return MF_KEY_Y;
        case GLFW_KEY_Z:                return MF_KEY_Z;

            // Function keys
        case GLFW_KEY_F1:               return MF_KEY_F1;
        case GLFW_KEY_F2:               return MF_KEY_F2;
        case GLFW_KEY_F3:               return MF_KEY_F3;
        case GLFW_KEY_F4:               return MF_KEY_F4;
        case GLFW_KEY_F5:               return MF_KEY_F5;
        case GLFW_KEY_F6:               return MF_KEY_F6;
        case GLFW_KEY_F7:               return MF_KEY_F7;
        case GLFW_KEY_F8:               return MF_KEY_F8;
        case GLFW_KEY_F9:               return MF_KEY_F9;
        case GLFW_KEY_F10:              return MF_KEY_F10;
        case GLFW_KEY_F11:              return MF_KEY_F11;
        case GLFW_KEY_F12:              return MF_KEY_F12;
            
        //                                return MF_KEY_F13;
        //                                return MF_KEY_F14;
        //                                return MF_KEY_F15;
        //                                return MF_KEY_F16;
        //                                return MF_KEY_F17;
        //                                return MF_KEY_F18;
        //                                return MF_KEY_F19;
        //                                return MF_KEY_F20;
        //                                return MF_KEY_F21;
        //                                return MF_KEY_F22;
        //                                return MF_KEY_F23;
        //                                return MF_KEY_F24;


            // Control keys
        case GLFW_KEY_ESCAPE:             return MF_KEY_ESCAPE;
        case GLFW_KEY_ENTER:              return MF_KEY_RETURN;
        case GLFW_KEY_TAB:                return MF_KEY_TAB;
        case GLFW_KEY_BACKSPACE:          return MF_KEY_BACKSPACE;
        case GLFW_KEY_INSERT:               return MF_KEY_INSERT;
        case GLFW_KEY_DELETE:               return MF_KEY_DELETE;
        case GLFW_KEY_RIGHT:                return MF_KEY_RIGHT;
        case GLFW_KEY_LEFT:                 return MF_KEY_LEFT;
        case GLFW_KEY_DOWN:                 return MF_KEY_DOWN;
        case GLFW_KEY_UP:                   return MF_KEY_UP;
        case GLFW_KEY_PAGE_UP:              return MF_KEY_PAGEUP;
        case GLFW_KEY_PAGE_DOWN:            return MF_KEY_PAGEDOWN;
        case GLFW_KEY_CAPS_LOCK:        return MF_KEY_CAPSLOCK;
        case GLFW_KEY_NUM_LOCK:         return MF_KEY_NUMLOCKCLEAR;
        case GLFW_KEY_SCROLL_LOCK:      return MF_KEY_SCROLLLOCK;
        case GLFW_KEY_PRINT_SCREEN:     return MF_KEY_PRINTSCREEN;
        case GLFW_KEY_PAUSE:            return MF_KEY_PAUSE;

            // Numeric keypad
        case GLFW_KEY_KP_0:               return MF_KEY_KP_0;
        case GLFW_KEY_KP_1:               return MF_KEY_KP_1;
        case GLFW_KEY_KP_2:                 return MF_KEY_KP_2;
        case GLFW_KEY_KP_3:                 return MF_KEY_KP_3;
        case GLFW_KEY_KP_4:                 return MF_KEY_KP_4;
        case GLFW_KEY_KP_5:                 return MF_KEY_KP_5;
        case GLFW_KEY_KP_6:                 return MF_KEY_KP_6;
        case GLFW_KEY_KP_7:                 return MF_KEY_KP_7;
        case GLFW_KEY_KP_8:                 return MF_KEY_KP_8;
        case GLFW_KEY_KP_9:                 return MF_KEY_KP_9;
        case GLFW_KEY_KP_DECIMAL:           return MF_KEY_KP_DECIMAL;
        case GLFW_KEY_KP_DIVIDE:            return MF_KEY_KP_DIVIDE;
        case GLFW_KEY_KP_MULTIPLY:          return MF_KEY_KP_MULTIPLY;
        case GLFW_KEY_KP_SUBTRACT:          return MF_KEY_KP_MINUS;
        case GLFW_KEY_KP_ADD:               return MF_KEY_KP_PLUS;
        case GLFW_KEY_KP_ENTER:             return MF_KEY_KP_ENTER;
        case GLFW_KEY_KP_EQUAL:             return MF_KEY_KP_EQUALS;

            // Modifier keys
        case GLFW_KEY_LEFT_SHIFT:         return MF_KEY_LSHIFT;
        case GLFW_KEY_LEFT_CONTROL:       return MF_KEY_LCTRL;
        case GLFW_KEY_LEFT_ALT:             return MF_KEY_LALT;
        case GLFW_KEY_LEFT_SUPER:           return MF_KEY_LSUPER;
        case GLFW_KEY_RIGHT_SHIFT:          return MF_KEY_RSHIFT;
        case GLFW_KEY_RIGHT_CONTROL:        return MF_KEY_RCTRL;
        case GLFW_KEY_RIGHT_ALT:            return MF_KEY_RALT;
        case GLFW_KEY_RIGHT_SUPER:          return MF_KEY_RSUPER;
        case GLFW_KEY_MENU:                 return MF_KEY_MENU;
            // ... Continue for other modifier keys

            // Add cases for any other keys you're interested in

        default: return MF_KEY_UNKNOWN;
        }
    }


    static u32 ToGLFWMouseButton(MouseButtons button) 
    {
        switch (button)
        {
            case MOUSE_BUTTON_NONE:
                break;

            case Magnefu::MOUSE_BUTTON_1:
                return GLFW_MOUSE_BUTTON_1;

            case Magnefu::MOUSE_BUTTON_2:
                return GLFW_MOUSE_BUTTON_2;

            case Magnefu::MOUSE_BUTTON_3:
                return GLFW_MOUSE_BUTTON_3;

            case Magnefu::MOUSE_BUTTON_4:
                return GLFW_MOUSE_BUTTON_4;

            case Magnefu::MOUSE_BUTTON_5:
                return GLFW_MOUSE_BUTTON_5;
                
            case Magnefu::MOUSE_BUTTON_6:
                return GLFW_MOUSE_BUTTON_6;

            case Magnefu::MOUSE_BUTTON_7:
                return GLFW_MOUSE_BUTTON_7;

            case Magnefu::MOUSE_BUTTON_8:
                return GLFW_MOUSE_BUTTON_8;

            default:
                break;
        }

        return u32_max;
    }

    cstring* GamepadAxisNames()
    {
        static cstring names[] = { "left_x", "left_y", "right_x", "right_y", "trigger_left", "trigger_right", "gamepad_axis_count" };
        return names;
    }

    cstring* GamepadButtonNames()
    {
        static cstring names[] = { "a", "b", "x", "y", "back", "guide", "start", "left_stick", "right_stick", "left_shoulder", "right_shoulder", "dpad_up", "dpad_down", "dpad_left", "dpad_right", "gamepad_button_count", };
        return names;
    }

    cstring* MouseButtonNames() {
        static cstring names[] = { "1", "2", "3", "4", "5", "6", "7", "8", "left", "right", "middle", "mouse_button_count", };
        return names;
    }

    cstring* KeyButtonNames() {
        static cstring names[] = { "unknown", "uuuu0", "uuuu1", "uuuu2", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "return", "escape", "backspace", "tab", "space", "minus", "equals", "leftbracket", "rightbracket", "backslash", "nonushash", "semicolon", "apostrophe", "grave", "comma", "period", "slash", "capslock", "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9", "f10", "f11", "f12", "printscreen", "scrolllock", "pause", "insert", "home", "pageup", "delete", "end", "pagedown", "right", "left", "down", "up", "numlockclear", "kp_divide", "kp_multiply", "kp_minus", "kp_plus", "kp_enter", "kp_1", "kp_2", "kp_3", "kp_4", "kp_5", "kp_6", "kp_7", "kp_8", "kp_9", "kp_0", "kp_period", "nonusbackslash", "application", "power", "kp_equals", "f13", "f14", "f15", "f16", "f17", "f18", "f19", "f20", "f21", "f22", "f23", "f24", "exe", "help", "menu", "select", "stop", "again", "undo", "cut", "copy", "paste", "find", "mute", "volumeup", "volumedown", "uuuu3", "uuuu4", "uuuu5", "kp_comma", "kp_equalsas400", "international1", "international2", "international3", "international4", "international5", "international6", "international7", "international8", "international9", "lang1", "lang2", "lang3", "lang4", "lang5", "lang6", "lang7", "lang8", "lang9", "alterase", "sysreq", "cancel", "clear", "prior", "return2", "separator", "out", "oper", "clearagain", "crsel", "exsel", "uuuu7", "uuuu8", "uuuu9", "uuuu10", "uuuu11", "uuuu12", "uuuu13", "uuuu14", "uuuu15", "uuuu16", "uuuu17", "kp_00", "kp_000", "thousandsseparator", "decimalseparator", "currencyunit", "currencysubunit", "kp_leftparen", "kp_rightparen", "kp_leftbrace", "kp_rightbrace", "kp_tab", "kp_backspace", "kp_a", "kp_b", "kp_c", "kp_d", "kp_e", "kp_f", "kp_xor", "kp_power", "kp_percent", "kp_less", "kp_greater", "kp_ampersand", "kp_dblampersand", "kp_verticalbar", "kp_dblverticalbar", "kp_colon", "kp_hash", "kp_space", "kp_at", "kp_exclam", "kp_memstore", "kp_memrecall", "kp_memclear", "kp_memadd", "kp_memsubtract", "kp_memmultiply", "kp_memdivide", "kp_plusminus", "kp_clear", "kp_clearentry", "kp_binary", "kp_octal", "kp_decimal", "kp_hexadecimal", "uuuu18", "uuuu19", "lctrl", "lshift", "lalt", "lgui", "rctrl", "rshift", "ralt", "rgui",  "uuuu7", "uuuu8", "uuuu9", "uuuu10", "uuuu11", "uuuu12", "uuuu13", "uuuu14", "uuuu15", "uuuu16", "uuuu17", "uuuu7", "uuuu8", "uuuu9", "uuuu10", "uuuu11", "uuuu12", "uuuu13", "uuuu14", "uuuu15", "uuuu16", "uuuu17", "uuuu14", "uuuu15", "uuuu16", "mode", "audionext", "audioprev", "audiostop", "audioplay", "audiomute", "mediaselect", "www", "mail", "calculator", "computer", "ac_search", "ac_home", "ac_back", "ac_forward", "ac_stop", "ac_refresh", "ac_bookmarks", "brightnessdown", "brightnessup", "displayswitch", "kbdillumtoggle", "kbdillumdown", "kbdillumup", "eject", "sleep", "app1", "app2", "audiorewind", "audiofastforward", "key_button_count" };

        return names;

    }

    void InputBackend::GetMouseState(InputVector2& position, u8* buttons, u32 num_buttons)
    {
        auto window = (WindowsWindow*)Application::Get()->GetWindow();
        double x, y;
        glfwGetCursorPos((GLFWwindow*)window->GetWindowHandle(), &x, &y);

        position.x = static_cast<f32>(x);
        position.y = static_cast<f32>(y);

        int mapped_glfw_button;
        int glfw_button;
        for (u32 i = 0; i < num_buttons; ++i) 
        {
            mapped_glfw_button = ToGLFWMouseButton((MouseButtons)i); // Implement this mapping
            glfw_button = glfwGetMouseButton((GLFWwindow*)window->GetWindowHandle(), mapped_glfw_button);
            buttons[i] = ( (glfw_button == GLFW_PRESS) || (glfw_button == GLFW_REPEAT) );
        }
    }


    void InputBackend::OnEvent(Event& event, u8* keys, u32 num_keys, Gamepad* gamepads, u32 num_gamepads, bool& has_focus)
    { 
        
        switch (event.GetEventType())
        {
            case Magnefu::EventType::KeyReleased:
            case Magnefu::EventType::KeyPressed: // TODO: (leon) add KeyPressed
            {
                KeyEvent& key_event = *(KeyEvent*)&event;

                i32 key = ToMFKeyCode(key_event.GetKeyCode());
                if (key >= 0 && key < (i32)num_keys)
                {
                    keys[key] = ( (key_event.GetAction() == GLFW_PRESS) || (key_event.GetAction() == GLFW_REPEAT) );
                    //MF_CORE_DEBUG("Message: {}\n Keycode: {} | Scancode: {} | Name: {}", key_event.ToString(), key, key_event.GetScancode(), KeyButtonNames()[key]);
                }
                    
                break;
            }
           
            
            // Mouse button updates handled by InputBackend::GetMouseState
            case Magnefu::EventType::MouseButtonPressed:
            {
                //MouseButtonPressedEvent& mouse_event = *(MouseButtonPressedEvent*)&event;
                break;
            }
            case Magnefu::EventType::MouseButtonReleased:
            {
                //MouseButtonReleasedEvent& mouse_event = *(MouseButtonReleasedEvent*)&event;
                break;
            }
            case Magnefu::EventType::MouseMoved:
                break;
            case Magnefu::EventType::MouseScrolled:
                break;

            case Magnefu::EventType::GamepadConnected:
            {
                MF_CORE_INFO("Gamepad Added");
                //int32_t index = 
                //
                //            init_gamepad(index, gamepads[index]);
                break;
            }
            case Magnefu::EventType::GamepadDisconnected:
                break;

            default:
                break;
        }

    }

#else

    // STUB implementation

    void InputBackend::init(Gamepad* gamepads, u32 num_gamepads) {
    }

    void InputBackend::shutdown() {
    }

    void InputBackend::on_event(void* event_, Gamepad* gamepads, u32 num_gamepads, bool& has_focus) {
    }

    void InputBackend::get_mouse_state(InputVector2& position, u8* buttons, u32 num_buttons) {
        position.x = FLT_MAX;
        position.y = FLT_MAX;
    }

#endif // INPUT_BACKEND_SDL


    Device DeviceFromPart(DevicePart part) 
    {
        switch (part) 
        {
            case DEVICE_PART_MOUSE:
            {
                return DEVICE_MOUSE;
            }

            case DEVICE_PART_GAMEPAD_AXIS:
            case DEVICE_PART_GAMEPAD_BUTTONS:
                //case InputBinding::GAMEPAD_HAT:
            {
                return DEVICE_GAMEPAD;
            }

            case DEVICE_PART_KEYBOARD:
            default:
            {
                return DEVICE_KEYBOARD;
            }
        }
    }


    



    // -- Input Service ----------------------------------------------------------- //
    static InputBackend s_input_backend;
    static InputService s_input_service;

    InputService* InputService::Instance()
    {
        return &s_input_service;
    }

	void InputService::Init(Allocator* allocator)
	{
        MF_CORE_INFO("InputService init");

        string_buffer.init(1000, allocator);
        action_maps.init(allocator, 16);
        actions.init(allocator, 64);
        bindings.init(allocator, 256);

        // Init gamepads handles
        for (size_t i = 0; i < k_max_gamepads; i++) 
        {
            gamepads[i].handle = nullptr;
        }

        memset(keys, 0, MF_KEY_COUNT);
        memset(previous_keys, 0, MF_KEY_COUNT);
        memset(mouse_buttons, 0, MOUSE_BUTTONS_COUNT);
        memset(previous_mouse_buttons, 0, MOUSE_BUTTONS_COUNT);

        s_input_backend.Init(gamepads, k_max_gamepads);
	}

    void InputService::Shutdown()
    {
        s_input_backend.Shutdown();
        action_maps.shutdown();
        actions.shutdown();
        bindings.shutdown();

        string_buffer.shutdown();

        MF_CORE_INFO("InputService shutdown");
    }

    static constexpr f32 k_mouse_drag_min_distance = 4.f;

    bool InputService::IsKeyDown(Keys key) 
    {
        return keys[key] && has_focus;
    }

    bool InputService::IsKeyJustPressed(Keys key, bool repeat) 
    {
        return keys[key] && !previous_keys[key] && has_focus;
    }

    bool InputService::IsKeyJustReleased(Keys key) 
    {
        return !keys[key] && previous_keys[key] && has_focus;
    }

    bool InputService::IsMouseDown(MouseButtons button) 
    {
        return mouse_buttons[button];
    }

    bool InputService::IsMouseClicked(MouseButtons button) 
    {
        return mouse_buttons[button] && !previous_mouse_buttons[button];
    }

    bool InputService::IsMouseReleased(MouseButtons button) 
    {
        return !mouse_buttons[button];
    }

    bool InputService::IsMouseDragging(MouseButtons button) 
    {
        if (!mouse_buttons[button])
            return false;

        return mouse_drag_distance[button] > k_mouse_drag_min_distance;
    }

    bool InputService::OnEvent(Event& event) 
    {
        s_input_backend.OnEvent(event, keys, MF_KEY_COUNT, gamepads, k_max_gamepads, has_focus);

        return true;
    }

    bool InputService::IsTriggered(InputHandle action) const {
        MF_CORE_ASSERT((action < actions.count()), "Action not available");
        return actions[action].Triggered();
    }

    f32 InputService::IsReadValue1D(InputHandle action) const 
    {
        MF_CORE_ASSERT((action < actions.count()), "Action not available");
        return actions[action].ReadValue1D();
    }

    InputVector2 InputService::IsReadValue2D(InputHandle action) const 
    {
        MF_CORE_ASSERT((action < actions.count()), "Action not available");
        return actions[action].ReadValue2D();
    }

    InputHandle InputService::CreateActionMap(const InputActionMapCreation& creation) 
    {
        InputActionMap new_action_map;
        new_action_map.active = creation.active;
        new_action_map.name = creation.name;

        action_maps.push(new_action_map);

        return action_maps.count() - 1;
    }

    InputHandle InputService::CreateAction(const InputActionCreation& creation) 
    {
        InputAction action;
        action.action_map = creation.action_map;
        action.name = creation.name;

        actions.push(action);

        return actions.count() - 1;
    }

    InputHandle InputService::FindActionMap(cstring name) const 
    {
        // TODO: move to hash map ?
        for (u32 i = 0; i < action_maps.count(); i++) 
        {
            //rprint( "%s, %s", name, action_maps[ i ].name );
            if (strcmp(name, action_maps[i].name) == 0) 
            {
                return i;
            }
        }
        return InputHandle(u32_max);
    }

    InputHandle InputService::FindAction(cstring name) const 
    {
        // TODO: move to hash map ?
        for (u32 i = 0; i < actions.count(); i++) 
        {
            //rprint( "%s, %s", name, actions[ i ].name );
            if (strcmp(name, actions[i].name) == 0) 
            {
                return i;
            }
        }
        return InputHandle(u32_max);
    }

    void InputService::AddButton(InputHandle action, DevicePart device_part, uint16_t button, bool repeat) 
    {
        const InputAction& binding_action = actions[action];

        InputBinding binding;
        binding.Set(BINDING_TYPE_BUTTON, DeviceFromPart(device_part), device_part, button, 0, 0, repeat).SetHandles(binding_action.action_map, action);

        bindings.push(binding);
    }

    void InputService::AddAxis1D(InputHandle action, DevicePart device_part, uint16_t axis, float min_deadzone, float max_deadzone) {
        const InputAction& binding_action = actions[action];

        InputBinding binding;
        binding.Set(BINDING_TYPE_AXIS_1D, DeviceFromPart(device_part), device_part, axis, 0, 0, 0).SetDeadZones(min_deadzone, max_deadzone).SetHandles(binding_action.action_map, action);

        bindings.push(binding);
    }

    void InputService::AddAxis2D(InputHandle action, DevicePart device_part, uint16_t x_axis, uint16_t y_axis, float min_deadzone, float max_deadzone) 
    {
        const InputAction& binding_action = actions[action];

        InputBinding binding, binding_x, binding_y;
        binding.Set(BINDING_TYPE_AXIS_2D, DeviceFromPart(device_part), device_part, u16_max, 1, 0, 0).SetDeadZones(min_deadzone, max_deadzone).SetHandles(binding_action.action_map, action);
        binding_x.Set(BINDING_TYPE_AXIS_2D, DeviceFromPart(device_part), device_part, x_axis, 0, 1, 0).SetDeadZones(min_deadzone, max_deadzone).SetHandles(binding_action.action_map, action);
        binding_y.Set(BINDING_TYPE_AXIS_2D, DeviceFromPart(device_part), device_part, y_axis, 0, 1, 0).SetDeadZones(min_deadzone, max_deadzone).SetHandles(binding_action.action_map, action);

        bindings.push(binding);
        bindings.push(binding_x);
        bindings.push(binding_y);
    }

    void InputService::AddVector1D(InputHandle action, DevicePart device_part_pos, uint16_t button_pos, DevicePart device_part_neg, uint16_t button_neg, bool repeat) 
    {
        const InputAction& binding_action = actions[action];

        InputBinding binding, binding_positive, binding_negative;
        binding.Set(BINDING_TYPE_VECTOR_1D, DeviceFromPart(device_part_pos), device_part_pos, u16_max, 1, 0, repeat).SetHandles(binding_action.action_map, action);
        binding_positive.Set(BINDING_TYPE_VECTOR_1D, DeviceFromPart(device_part_pos), device_part_pos, button_pos, 0, 1, repeat).SetHandles(binding_action.action_map, action);
        binding_negative.Set(BINDING_TYPE_VECTOR_1D, DeviceFromPart(device_part_neg), device_part_neg, button_neg, 0, 1, repeat).SetHandles(binding_action.action_map, action);

        bindings.push(binding);
        bindings.push(binding_positive);
        bindings.push(binding_negative);
    }

    void InputService::AddVector2D(InputHandle action, DevicePart device_part_up, uint16_t button_up, DevicePart device_part_down, uint16_t button_down, DevicePart device_part_left, uint16_t button_left, DevicePart device_part_right, uint16_t button_right, bool repeat) 
    {
        const InputAction& binding_action = actions[action];

        InputBinding binding, binding_up, binding_down, binding_left, binding_right;

        binding.Set(BINDING_TYPE_VECTOR_2D, DeviceFromPart(device_part_up), device_part_up, u16_max, 1, 0, repeat).SetHandles(binding_action.action_map, action);
        binding_up.Set(BINDING_TYPE_VECTOR_2D, DeviceFromPart(device_part_up), device_part_up, button_up, 0, 1, repeat).SetHandles(binding_action.action_map, action);
        binding_down.Set(BINDING_TYPE_VECTOR_2D, DeviceFromPart(device_part_down), device_part_down, button_down, 0, 1, repeat).SetHandles(binding_action.action_map, action);
        binding_left.Set(BINDING_TYPE_VECTOR_2D, DeviceFromPart(device_part_left), device_part_left, button_left, 0, 1, repeat).SetHandles(binding_action.action_map, action);
        binding_right.Set(BINDING_TYPE_VECTOR_2D, DeviceFromPart(device_part_right), device_part_right, button_right, 0, 1, repeat).SetHandles(binding_action.action_map, action);

        bindings.push(binding);
        bindings.push(binding_up);
        bindings.push(binding_down);
        bindings.push(binding_left);
        bindings.push(binding_right);

    }

    void InputService::SetEventCallback(const EventCallbackFn& callback)
    {
        for (u32 gamepad = 0; gamepad < k_max_gamepads; gamepad++)
        {
            gamepads[gamepad].EventCallback = callback;
        }
    }

    void InputService::NewFrame() 
    {
        // Cache previous frame keys.
        // Resetting previous frame breaks key pressing - there can be more frames between key presses even if continuously pressed.
        for (u32 i = 0; i < MF_KEY_COUNT; ++i) 
        {
            previous_keys[i] = keys[i];
            //keys[ i ] = 0;
        }

        for (u32 i = 0; i < MOUSE_BUTTONS_COUNT; ++i) 
        {
            previous_mouse_buttons[i] = mouse_buttons[i];
        }

        for (u32 i = 0; i < k_max_gamepads; ++i) 
        {
            if (gamepads[i].IsAttached()) 
            {
                for (u32 k = 0; k < GAMEPAD_BUTTON_COUNT; k++) 
                {
                    gamepads[i].previous_buttons[k] = gamepads[i].buttons[k];
                }
            }
        }
    }

    void InputService::Update(f32 delta) 
    {

        // Update Mouse ------------------------------------------------------- //
        previous_mouse_position = mouse_position;
        // Update current mouse state
        s_input_backend.GetMouseState(mouse_position, mouse_buttons, MOUSE_BUTTONS_COUNT);

        for (u32 i = 0; i < MOUSE_BUTTONS_COUNT; ++i) 
        {
            // Just clicked. Save position
            if (IsMouseClicked((MouseButtons)i)) 
            {
                mouse_clicked_position[i] = mouse_position;
            }
            else if (IsMouseDown((MouseButtons)i)) 
            {
                f32 delta_x = mouse_position.x - mouse_clicked_position[i].x;
                f32 delta_y = mouse_position.y - mouse_clicked_position[i].y;
                mouse_drag_distance[i] = sqrtf((delta_x * delta_x) + (delta_y * delta_y));
            }
        }

        // NEW UPDATE

        // Update all actions maps
        // Update all actions
        // Scan each action of the map
        for (u32 j = 0; j < actions.count(); j++) 
        {
            InputAction& input_action = actions[j];
            input_action.value = { 0,0 };
        }

        // Read all input values for each binding
        // First get all the button or composite parts. Composite input will be calculated after.
        for (u32 k = 0; k < bindings.count(); k++) 
        {
            InputBinding& input_binding = bindings[k];
            // Skip composite bindings. Their value will be calculated after.
            if (input_binding.is_composite)
                continue;

            input_binding.value = false;

            switch (input_binding.device) 
            {
                case DEVICE_KEYBOARD:
                {
                    bool key_value = input_binding.repeat ? IsKeyDown((Keys)input_binding.button) : IsKeyJustPressed((Keys)input_binding.button, false);
                    input_binding.value = key_value ? 1.0f : 0.0f;
                    break;
                }

                case DEVICE_GAMEPAD:
                {
                    Gamepad& gamepad = gamepads[0];
                    if (gamepad.handle == nullptr)
                    {
                        break;
                    }

                    const float min_deadzone = input_binding.min_deadzone;
                    const float max_deadzone = input_binding.max_deadzone;

                    switch (input_binding.device_part) 
                    {
                        case DEVICE_PART_GAMEPAD_AXIS:
                        {
                            input_binding.value = gamepad.axis[input_binding.button];
                            input_binding.value = fabs(input_binding.value) < min_deadzone ? 0.0f : input_binding.value;
                            input_binding.value = fabs(input_binding.value) > max_deadzone ? (input_binding.value < 0 ? -1.0f : 1.0f) : input_binding.value;

                            break;
                        }
                        case DEVICE_PART_GAMEPAD_BUTTONS:
                        {
                            //input_binding.value = gamepad.buttons[ input_binding.button ];
                            input_binding.value = input_binding.repeat ? gamepad.IsButtonDown((GamepadButtons)input_binding.button) : gamepad.IsButtonJustPressed((GamepadButtons)input_binding.button);
                            break;
                        }
                        /*case InputBinding::GAMEPAD_HAT:
                        {
                            input_binding.value = gamepad.hats[ input_binding.button ];
                            break;
                        }*/

                    }
                }
            }
        }

        for (u32 k = 0; k < bindings.count(); k++) 
        {
            InputBinding& input_binding = bindings[k];

            if (input_binding.is_part_of_composite)
                continue;

            InputAction& input_action = actions[input_binding.action_index];

            switch (input_binding.type) 
            {
                case BINDING_TYPE_BUTTON:
                {
                    input_action.value.x = fmax(input_action.value.x, input_binding.value ? 1.0f : 0.0f);
                    break;
                }

                case BINDING_TYPE_AXIS_1D:
                {
                    input_action.value.x = input_binding.value != 0.f ? input_binding.value : input_action.value.x;
                    break;
                }

                case BINDING_TYPE_AXIS_2D:
                {
                    // Retrieve following 2 bindings
                    InputBinding& input_binding_x = bindings[++k];
                    InputBinding& input_binding_y = bindings[++k];

                    input_action.value.x = input_binding_x.value != 0.0f ? input_binding_x.value : input_action.value.x;
                    input_action.value.y = input_binding_y.value != 0.0f ? input_binding_y.value : input_action.value.y;

                    break;
                }

                case BINDING_TYPE_VECTOR_1D:
                {
                    // Retrieve following 2 bindings
                    InputBinding& input_binding_pos = bindings[++k];
                    InputBinding& input_binding_neg = bindings[++k];

                    input_action.value.x = input_binding_pos.value ? input_binding_pos.value : input_binding_neg.value ? -input_binding_neg.value : input_action.value.x;
                    break;
                }

                case BINDING_TYPE_VECTOR_2D:
                {
                    // Retrieve following 4 bindings
                    InputBinding& input_binding_up = bindings[++k];
                    InputBinding& input_binding_down = bindings[++k];
                    InputBinding& input_binding_left = bindings[++k];
                    InputBinding& input_binding_right = bindings[++k];

                    input_action.value.x = input_binding_right.value ? 1.0f : input_binding_left.value ? -1.0f : input_action.value.x;
                    input_action.value.y = input_binding_up.value ? 1.0f : input_binding_down.value ? -1.0f : input_action.value.y;
                    break;
                }
            }
        }


        // Update all Input Actions ////////////////////////////
        // TODO: flat all arrays
        // Scan each action map
        /*
        for ( u32 i = 0; i < input_action_maps.size; i++ ) {
            InputActionMap& action_map = input_action_maps[ i ];
            if ( !action_map.active ) {
                continue;
            }

            // Scan each action of the map
            for ( u32 j = 0; j < action_map.num_actions; j++ ) {
                InputAction& input_action = action_map.actions[ j ];

                // First get all the button or composite parts. Composite input will be calculated after.
                for ( u32 k = 0; k < input_action.bindings.size; k++ ) {
                    InputBinding& input_binding = input_action.bindings[ k ];
                    // Skip composite bindings. Their value will be calculated after.
                    if ( input_binding.is_composite )
                        continue;

                    input_binding.value = false;

                    switch ( input_binding.device ) {
                        case DEVICE_KEYBOARD:
                        {
                            bool key_value = input_binding.repeat ? is_key_down( (Keys)input_binding.button ) : is_key_just_pressed( (Keys)input_binding.button, false );
                            input_binding.value = key_value ? 1.0f : 0.0f;
                            break;
                        }

                        case DEVICE_GAMEPAD:
                        {
                            Gamepad& gamepad = gamepads[ 0 ];
                            if ( gamepad.handle == nullptr ) {
                                break;
                            }

                            const float min_deadzone = input_binding.min_deadzone;
                            const float max_deadzone = input_binding.max_deadzone;

                            switch ( input_binding.device_part ) {
                                case GAMEPAD_AXIS:
                                {
                                    input_binding.value = gamepad.axis[ input_binding.button ];
                                    input_binding.value = fabs( input_binding.value ) < min_deadzone ? 0.0f : input_binding.value;
                                    input_binding.value = fabs( input_binding.value ) > max_deadzone ? ( input_binding.value < 0 ? -1.0f : 1.0f ) : input_binding.value;

                                    break;
                                }
                                case GAMEPAD_BUTTONS:
                                {
                                    input_binding.value = gamepad.buttons[ input_binding.button ];
                                    break;
                                }
                                /*case InputBinding::GAMEPAD_HAT:
                                {
                                    input_binding.value = gamepad.hats[ input_binding.button ];
                                    break;
                                }* /
                            }

                            break;
                        }
                    }
                }

                // Calculate/syntethize composite input values into input action
                input_action.value = { 0,0 };

                for ( u32 k = 0; k < input_action.bindings.size; k++ ) {
                    InputBinding& input_binding = input_action.bindings[ k ];

                    if ( input_binding.is_part_of_composite )
                        continue;

                    switch ( input_binding.type ) {
                        case BUTTON:
                        {
                            input_action.value.x = fmax( input_action.value.x, input_binding.value ? 1.0f : 0.0f );
                            break;
                        }

                        case AXIS_1D:
                        {
                            input_action.value.x = input_binding.value != 0.f ? input_binding.value : input_action.value.x;
                            break;
                        }

                        case AXIS_2D:
                        {
                            // Retrieve following 2 bindings
                            InputBinding& input_binding_x = input_action.bindings[ ++k ];
                            InputBinding& input_binding_y = input_action.bindings[ ++k ];

                            input_action.value.x = input_binding_x.value != 0.0f ? input_binding_x.value : input_action.value.x;
                            input_action.value.y = input_binding_y.value != 0.0f ? input_binding_y.value : input_action.value.y;

                            break;
                        }

                        case VECTOR_1D:
                        {
                            // Retrieve following 2 bindings
                            InputBinding& input_binding_pos = input_action.bindings[ ++k ];
                            InputBinding& input_binding_neg = input_action.bindings[ ++k ];

                            input_action.value.x = input_binding_pos.value ? input_binding_pos.value : input_binding_neg.value ? -input_binding_neg.value : input_action.value.x;
                            break;
                        }

                        case VECTOR_2D:
                        {
                            // Retrieve following 4 bindings
                            InputBinding& input_binding_up = input_action.bindings[ ++k ];
                            InputBinding& input_binding_down = input_action.bindings[ ++k ];
                            InputBinding& input_binding_left = input_action.bindings[ ++k ];
                            InputBinding& input_binding_right = input_action.bindings[ ++k ];

                            input_action.value.x = input_binding_right.value ? 1.0f : input_binding_left.value ? -1.0f : input_action.value.x;
                            input_action.value.y = input_binding_up.value ? 1.0f : input_binding_down.value ? -1.0f : input_action.value.y;
                            break;
                        }
                    }
                }
            }
        }
        */
    }

//
    
    void InputService::DebugUI() 
    {


        if (ImGui::Begin("Input")) 
        {
            ImGui::Text("Has focus %u", has_focus ? 1 : 0);

            if (ImGui::TreeNode("Devices")) 
            {
                ImGui::Separator();
                if (ImGui::TreeNode("Gamepads")) 
                {
                    for (u32 i = 0; i < k_max_gamepads; ++i) 
                    {
                        const Gamepad& g = gamepads[i];
                        ImGui::Text("Name: %s, id %d, index %u", g.name, g.id, g.index);
                        // Attached gamepad
                        if (g.IsAttached()) 
                        {
                            ImGui::NewLine();
                            ImGui::Columns(GAMEPAD_AXIS_COUNT);
                            for (u32 gi = 0; gi < GAMEPAD_AXIS_COUNT; gi++) 
                            {
                                ImGui::Text("%s", GamepadAxisNames()[gi]);
                                ImGui::NextColumn();
                            }
                            for (u32 gi = 0; gi < GAMEPAD_AXIS_COUNT; gi++) 
                            {
                                ImGui::Text("%f", g.axis[gi]);
                                ImGui::NextColumn();
                            }
                            ImGui::NewLine();
                            ImGui::Columns(GAMEPAD_BUTTON_COUNT);
                            for (u32 gi = 0; gi < GAMEPAD_BUTTON_COUNT; gi++) 
                            {
                                ImGui::Text("%s", GamepadButtonNames()[gi]);
                                ImGui::NextColumn();
                            }
                            ImGui::Columns(GAMEPAD_BUTTON_COUNT);
                            for (u32 gi = 0; gi < GAMEPAD_BUTTON_COUNT; gi++) 
                            {
                                ImGui::Text("%u", g.buttons[gi]);
                                ImGui::NextColumn();
                            }

                            ImGui::Columns(1);
                        }
                        ImGui::Separator();
                    }
                    ImGui::TreePop();
                }

                ImGui::Separator();
                if (ImGui::TreeNode("Mouse")) 
                {
                    ImGui::Text("Position     %f,%f", mouse_position.x, mouse_position.y);
                    ImGui::Text("Previous pos %f,%f", previous_mouse_position.x, previous_mouse_position.y);

                    ImGui::Separator();

                    for (u32 i = 0; i < MOUSE_BUTTONS_COUNT; i++) {
                        ImGui::Text("Button %u", i);
                        ImGui::SameLine();
                        ImGui::Text("Clicked Position     %4.1f,%4.1f", mouse_clicked_position[i].x, mouse_clicked_position[i].y);
                        ImGui::SameLine();
                        ImGui::Text("Button %u, Previous %u", mouse_buttons[i], previous_mouse_buttons[i]);
                        ImGui::SameLine();
                        ImGui::Text("Drag %f", mouse_drag_distance[i]);

                        ImGui::Separator();
                    }
                    ImGui::TreePop();
                }

                ImGui::Separator();
                if (ImGui::TreeNode("Keyboard")) 
                {
                    for (u32 i = 0; i < MF_KEY_COUNT; i++) 
                    {
                        // There are gaps in enum. Need to check if key exist
                        ImGui::Text("Button %u", i);
                        ImGui::SameLine();
                        ImGui::Text("Button %u, Previous %u, Name %s", keys[i], previous_keys[i], KeyButtonNames()[i]);
                    }
                    ImGui::TreePop();
                }
                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Actions")) 
            {

                for (u32 j = 0; j < actions.count(); j++) 
                {
                    const InputAction& input_action = actions[j];
                    ImGui::Text("Action %s, x %2.3f y %2.3f", input_action.name, input_action.value.x, input_action.value.y);
                }

                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Bindings")) 
            {
                for (u32 k = 0; k < bindings.count(); k++) 
                {
                    const InputBinding& binding = bindings[k];
                    const InputAction& parent_action = actions[binding.action_index];

                    cstring button_name = "";
                    switch (binding.device_part) 
                    {
                        case DEVICE_PART_KEYBOARD:
                        {
                            button_name = KeyButtonNames()[binding.button];
                            break;
                        }
                        case DEVICE_PART_MOUSE:
                        {
                            break;
                        }
                        case DEVICE_PART_GAMEPAD_AXIS:
                        {
                            break;
                        }
                        case DEVICE_PART_GAMEPAD_BUTTONS:
                        {
                            break;
                        }
                    }

                    switch (binding.type) 
                    {
                        case BINDING_TYPE_VECTOR_1D:
                        {
                            ImGui::Text("Binding action %s, type %s, value %f, composite %u, part of composite %u, button %s", parent_action.name, "vector 1d", binding.value, binding.is_composite, binding.is_part_of_composite, button_name);
                            break;
                        }
                        case BINDING_TYPE_VECTOR_2D:
                        {
                            ImGui::Text("Binding action %s, type %s, value %f, composite %u, part of composite %u", parent_action.name, "vector 2d", binding.value, binding.is_composite, binding.is_part_of_composite);
                            break;
                        }
                        case BINDING_TYPE_AXIS_1D:
                        {
                            ImGui::Text("Binding action %s, type %s, value %f, composite %u, part of composite %u", parent_action.name, "axis 1d", binding.value, binding.is_composite, binding.is_part_of_composite);
                            break;
                        }
                        case BINDING_TYPE_AXIS_2D:
                        {
                            ImGui::Text("Binding action %s, type %s, value %f, composite %u, part of composite %u", parent_action.name, "axis 2d", binding.value, binding.is_composite, binding.is_part_of_composite);
                            break;
                        }
                        case BINDING_TYPE_BUTTON:
                        {
                            ImGui::Text("Binding action %s, type %s, value %f, composite %u, part of composite %u, button %s", parent_action.name, "button", binding.value, binding.is_composite, binding.is_part_of_composite, button_name);
                            break;
                        }
                    }
                }

                ImGui::TreePop();
            }

        }
        ImGui::End();
    }

    // InputAction /////////////////////////////////////////////////////////

    bool InputAction::Triggered() const 
    {
        return value.x != 0.0f;
    }

    float InputAction::ReadValue1D() const 
    {
        return value.x;
    }

    InputVector2 InputAction::ReadValue2D() const 
    {
        return value;
    }


    InputBinding& InputBinding::Set(BindingType type_, Device device_, DevicePart device_part_, u16 button_, u8 is_composite_, u8 is_part_of_composite_, u8 repeat_) {
        type = type_;
        device = device_;
        device_part = device_part_;
        button = button_;
        is_composite = is_composite_;
        is_part_of_composite = is_part_of_composite_;
        repeat = repeat_;
        return *this;
    }

    InputBinding& InputBinding::SetDeadZones(f32 min, f32 max) 
    {
        min_deadzone = min;
        max_deadzone = max;
        return *this;
    }

    InputBinding& InputBinding::SetHandles(InputHandle action_map, InputHandle action) 
    {
        // Don't expect this to have more than 256.
        MF_CORE_ASSERT((action_map < 256), "Exceeded allotted number of action maps");
        MF_CORE_ASSERT((action < 16636), "Exceeded allotted number of actions");

        action_map_index = (u8)action_map;
        action_index = (u16)action;

        return *this;
    }
}