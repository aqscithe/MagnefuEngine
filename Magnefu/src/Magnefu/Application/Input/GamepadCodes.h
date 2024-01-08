#pragma once

static const uint32_t               k_max_gamepads = 4;

namespace Magnefu
{
    enum GamepadAxis
    {
        GAMEPAD_AXIS_LEFTX = 0,
        GAMEPAD_AXIS_LEFTY,
        GAMEPAD_AXIS_RIGHTX,
        GAMEPAD_AXIS_RIGHTY,
        GAMEPAD_AXIS_TRIGGERLEFT,
        GAMEPAD_AXIS_TRIGGERRIGHT,
        GAMEPAD_AXIS_COUNT
    }; // enum GamepadAxis

   

    enum GamepadButtons 
    {
        GAMEPAD_BUTTON_A = 0,
        GAMEPAD_BUTTON_B,
        GAMEPAD_BUTTON_X,
        GAMEPAD_BUTTON_Y,
        GAMEPAD_BUTTON_BACK,
        GAMEPAD_BUTTON_GUIDE,
        GAMEPAD_BUTTON_START,
        GAMEPAD_BUTTON_LEFTSTICK,
        GAMEPAD_BUTTON_RIGHTSTICK,
        GAMEPAD_BUTTON_LEFTSHOULDER,
        GAMEPAD_BUTTON_RIGHTSHOULDER,
        GAMEPAD_BUTTON_DPAD_UP,
        GAMEPAD_BUTTON_DPAD_DOWN,
        GAMEPAD_BUTTON_DPAD_LEFT,
        GAMEPAD_BUTTON_DPAD_RIGHT,
        GAMEPAD_BUTTON_COUNT
    }; // enum GamepadButtons

}
