#pragma once


#include "Magnefu/Core/Platform.hpp"
#include "Magnefu/Core/Assertions.h"


namespace Magnefu
{


	enum Keys
	{
        MF_KEY_UNKNOWN = 0,

        MF_KEY_A = 4,
        MF_KEY_B = 5,
        MF_KEY_C = 6,
        MF_KEY_D = 7,
        MF_KEY_E = 8,
        MF_KEY_F = 9,
        MF_KEY_G = 10,
        MF_KEY_H = 11,
        MF_KEY_I = 12,
        MF_KEY_J = 13,
        MF_KEY_K = 14,
        MF_KEY_L = 15,
        MF_KEY_M = 16,
        MF_KEY_N = 17,
        MF_KEY_O = 18,
        MF_KEY_P = 19,
        MF_KEY_Q = 20,
        MF_KEY_R = 21,
        MF_KEY_S = 22,
        MF_KEY_T = 23,
        MF_KEY_U = 24,
        MF_KEY_V = 25,
        MF_KEY_W = 26,
        MF_KEY_X = 27,
        MF_KEY_Y = 28,
        MF_KEY_Z = 29,

        MF_KEY_1 = 30,
        MF_KEY_2 = 31,
        MF_KEY_3 = 32,
        MF_KEY_4 = 33,
        MF_KEY_5 = 34,
        MF_KEY_6 = 35,
        MF_KEY_7 = 36,
        MF_KEY_8 = 37,
        MF_KEY_9 = 38,
        MF_KEY_0 = 39,

        MF_KEY_RETURN = 40,
        MF_KEY_ESCAPE = 41,
        MF_KEY_BACKSPACE = 42,
        MF_KEY_TAB = 43,
        MF_KEY_SPACE = 44,

        MF_KEY_MINUS = 45,
        MF_KEY_EQUALS = 46,
        MF_KEY_LEFTBRACKET = 47,
        MF_KEY_RIGHTBRACKET = 48,
        MF_KEY_BACKSLASH = 49,
        MF_KEY_NONUSHASH = 50,
        MF_KEY_SEMICOLON = 51,
        MF_KEY_APOSTROPHE = 52,
        MF_KEY_GRAVE = 53,
        MF_KEY_COMMA = 54,
        MF_KEY_PERIOD = 55,
        MF_KEY_SLASH = 56,

        MF_KEY_CAPSLOCK = 57,

        MF_KEY_F1 = 58,
        MF_KEY_F2 = 59,
        MF_KEY_F3 = 60,
        MF_KEY_F4 = 61,
        MF_KEY_F5 = 62,
        MF_KEY_F6 = 63,
        MF_KEY_F7 = 64,
        MF_KEY_F8 = 65,
        MF_KEY_F9 = 66,
        MF_KEY_F10 = 67,
        MF_KEY_F11 = 68,
        MF_KEY_F12 = 69,

        MF_KEY_PRINTSCREEN = 70,
        MF_KEY_SCROLLLOCK = 71,
        MF_KEY_PAUSE = 72,
        MF_KEY_INSERT = 73,
        MF_KEY_HOME = 74,
        MF_KEY_PAGEUP = 75,
        MF_KEY_DELETE = 76,
        MF_KEY_END = 77,
        MF_KEY_PAGEDOWN = 78,
        MF_KEY_RIGHT = 79,
        MF_KEY_LEFT = 80,
        MF_KEY_DOWN = 81,
        MF_KEY_UP = 82,

        MF_KEY_NUMLOCKCLEAR = 83,
        MF_KEY_KP_DIVIDE = 84,
        MF_KEY_KP_MULTIPLY = 85,
        MF_KEY_KP_MINUS = 86,
        MF_KEY_KP_PLUS = 87,
        MF_KEY_KP_ENTER = 88,
        MF_KEY_KP_1 = 89,
        MF_KEY_KP_2 = 90,
        MF_KEY_KP_3 = 91,
        MF_KEY_KP_4 = 92,
        MF_KEY_KP_5 = 93,
        MF_KEY_KP_6 = 94,
        MF_KEY_KP_7 = 95,
        MF_KEY_KP_8 = 96,
        MF_KEY_KP_9 = 97,
        MF_KEY_KP_0 = 98,
        MF_KEY_KP_PERIOD = 99,

        MF_KEY_NONUSBACKSLASH = 100,
        MF_KEY_APPLICATION = 101,
        MF_KEY_POWER = 102,
        MF_KEY_KP_EQUALS = 103,
        MF_KEY_F13 = 104,
        MF_KEY_F14 = 105,
        MF_KEY_F15 = 106,
        MF_KEY_F16 = 107,
        MF_KEY_F17 = 108,
        MF_KEY_F18 = 109,
        MF_KEY_F19 = 110,
        MF_KEY_F20 = 111,
        MF_KEY_F21 = 112,
        MF_KEY_F22 = 113,
        MF_KEY_F23 = 114,
        MF_KEY_F24 = 115,
        MF_KEY_EXE = 116,
        MF_KEY_HELP = 117,
        MF_KEY_MENU = 118,
        MF_KEY_SELECT = 119,
        MF_KEY_STOP = 120,
        MF_KEY_AGAIN = 121,
        MF_KEY_UNDO = 122,
        MF_KEY_CUT = 123,
        MF_KEY_COPY = 124,
        MF_KEY_PASTE = 125,
        MF_KEY_FIND = 126,
        MF_KEY_MUTE = 127,
        MF_KEY_VOLUMEUP = 128,
        MF_KEY_VOLUMEDOWN = 129,

        MF_KEY_KP_COMMA = 133,
        MF_KEY_KP_EQUALSAS400 = 134,

        MF_KEY_INTERNATIONAL1 = 135,
        MF_KEY_INTERNATIONAL2 = 136,
        MF_KEY_INTERNATIONAL3 = 137, /**< Yen */
        MF_KEY_INTERNATIONAL4 = 138,
        MF_KEY_INTERNATIONAL5 = 139,
        MF_KEY_INTERNATIONAL6 = 140,
        MF_KEY_INTERNATIONAL7 = 141,
        MF_KEY_INTERNATIONAL8 = 142,
        MF_KEY_INTERNATIONAL9 = 143,
        MF_KEY_LANG1 = 144, /**< Hangul/English toggle */
        MF_KEY_LANG2 = 145, /**< Hanja conversion */
        MF_KEY_LANG3 = 146, /**< Katakana */
        MF_KEY_LANG4 = 147, /**< Hiragana */
        MF_KEY_LANG5 = 148, /**< Zenkaku/Hankaku */
        MF_KEY_LANG6 = 149, /**< reserved */
        MF_KEY_LANG7 = 150, /**< reserved */
        MF_KEY_LANG8 = 151, /**< reserved */
        MF_KEY_LANG9 = 152, /**< reserved */

        MF_KEY_ALTERASE = 153, /**< Erase-Eaze */
        MF_KEY_SYSREQ = 154,
        MF_KEY_CANCEL = 155,
        MF_KEY_CLEAR = 156,
        MF_KEY_PRIOR = 157,
        MF_KEY_RETURN2 = 158,
        MF_KEY_SEPARATOR = 159,
        MF_KEY_OUT = 160,
        MF_KEY_OPER = 161,
        MF_KEY_CLEARAGAIN = 162,
        MF_KEY_CRSEL = 163,
        MF_KEY_EXSEL = 164,

        MF_KEY_KP_00 = 176,
        MF_KEY_KP_000 = 177,
        MF_KEY_THOUSANDSSEPARATOR = 178,
        MF_KEY_DECIMALSEPARATOR = 179,
        MF_KEY_CURRENCYUNIT = 180,
        MF_KEY_CURRENCYSUBUNIT = 181,
        MF_KEY_KP_LEFTPAREN = 182,
        MF_KEY_KP_RIGHTPAREN = 183,
        MF_KEY_KP_LEFTBRACE = 184,
        MF_KEY_KP_RIGHTBRACE = 185,
        MF_KEY_KP_TAB = 186,
        MF_KEY_KP_BACKSPACE = 187,
        MF_KEY_KP_A = 188,
        MF_KEY_KP_B = 189,
        MF_KEY_KP_C = 190,
        MF_KEY_KP_D = 191,
        MF_KEY_KP_E = 192,
        MF_KEY_KP_F = 193,
        MF_KEY_KP_XOR = 194,
        MF_KEY_KP_POWER = 195,
        MF_KEY_KP_PERCENT = 196,
        MF_KEY_KP_LESS = 197,
        MF_KEY_KP_GREATER = 198,
        MF_KEY_KP_AMPERSAND = 199,
        MF_KEY_KP_DBLAMPERSAND = 200,
        MF_KEY_KP_VERTICALBAR = 201,
        MF_KEY_KP_DBLVERTICALBAR = 202,
        MF_KEY_KP_COLON = 203,
        MF_KEY_KP_HASH = 204,
        MF_KEY_KP_SPACE = 205,
        MF_KEY_KP_AT = 206,
        MF_KEY_KP_EXCLAM = 207,
        MF_KEY_KP_MEMSTORE = 208,
        MF_KEY_KP_MEMRECALL = 209,
        MF_KEY_KP_MEMCLEAR = 210,
        MF_KEY_KP_MEMADD = 211,
        MF_KEY_KP_MEMSUBTRACT = 212,
        MF_KEY_KP_MEMMULTIPLY = 213,
        MF_KEY_KP_MEMDIVIDE = 214,
        MF_KEY_KP_PLUSMINUS = 215,
        MF_KEY_KP_CLEAR = 216,
        MF_KEY_KP_CLEARENTRY = 217,
        MF_KEY_KP_BINARY = 218,
        MF_KEY_KP_OCTAL = 219,
        MF_KEY_KP_DECIMAL = 220,
        MF_KEY_KP_HEXADECIMAL = 221,

        MF_KEY_LCTRL = 224,
        MF_KEY_LSHIFT = 225,
        MF_KEY_LALT = 226, /**< alt, option */
        MF_KEY_LSUPER = 227, /**< windows, command (apple), meta */
        MF_KEY_RCTRL = 228,
        MF_KEY_RSHIFT = 229,
        MF_KEY_RALT = 230, /**< alt gr, option */
        MF_KEY_RSUPER = 231, /**< windows, command (apple), meta */

        MF_KEY_MODE = 257,    /**< I'm not sure if this is really not covered
        *   by any of the above, but since there's a
        *   special KMOD_MODE for it I'm adding it here
        */

        MF_KEY_AUDIONEXT = 258,
        MF_KEY_AUDIOPREV = 259,
        MF_KEY_AUDIOSTOP = 260,
        MF_KEY_AUDIOPLAY = 261,
        MF_KEY_AUDIOMUTE = 262,
        MF_KEY_MEDIASELECT = 263,
        MF_KEY_WWW = 264,
        MF_KEY_MAIL = 265,
        MF_KEY_CALCULATOR = 266,
        MF_KEY_COMPUTER = 267,
        MF_KEY_AC_SEARCH = 268,
        MF_KEY_AC_HOME = 269,
        MF_KEY_AC_BACK = 270,
        MF_KEY_AC_FORWARD = 271,
        MF_KEY_AC_STOP = 272,
        MF_KEY_AC_REFRESH = 273,
        MF_KEY_AC_BOOKMARKS = 274,

        MF_KEY_BRIGHTNESSDOWN = 275,
        MF_KEY_BRIGHTNESSUP = 276,
        MF_KEY_DISPLAYSWITCH = 277, /**< display mirroring/dual display
        switch, video mode switch */
        MF_KEY_KBDILLUMTOGGLE = 278,
        MF_KEY_KBDILLUMDOWN = 279,
        MF_KEY_KBDILLUMUP = 280,
        MF_KEY_EJECT = 281,
        MF_KEY_SLEEP = 282,

        MF_KEY_APP1 = 283,
        MF_KEY_APP2 = 284,

        MF_KEY_AUDIOREWIND = 285,
        MF_KEY_AUDIOFASTFORWARD = 286,

        MF_KEY_LAST = MF_KEY_AUDIOFASTFORWARD,
        MF_KEY_COUNT
        //MF_KEY_COUNT = 512

	};

	enum KeyMods
	{
		MF_MOD_SHIFT = BIT(1),
		MF_MOD_CONTROL = BIT(2),
		MF_MOD_ALT = BIT(3),
		MF_MOD_SUPER = BIT(4),
		MF_MOD_CAPS_LOCK = BIT(5),
		MF_MOD_NUM_LOCK = BIT(6),
	};
}



/*! @} */

/*! @defgroup mods Modifier key flags
 *  @brief Modifier key flags.
 *
 *  See [key input](@ref input_key) for how these are used.
 *
 *  @ingroup input
 *  @{ */

/*! @brief If this bit is set one or more Shift keys were held down.
 *
 *  If this bit is set one or more Shift keys were held down.
 */
//#define MF_MOD_SHIFT           0x0001
/*! @brief If this bit is set one or more Control keys were held down.
 *
 *  If this bit is set one or more Control keys were held down.
 */
//#define MF_MOD_CONTROL         0x0002
   /*! @brief If this bit is set one or more Alt keys were held down.
	*
	*  If this bit is set one or more Alt keys were held down.
	*/
//#define MF_MOD_ALT             0x0004
	/*! @brief If this bit is set one or more Super keys were held down.
	 *
	 *  If this bit is set one or more Super keys were held down.
	 */
//#define MF_MOD_SUPER           0x0008
	 /*! @brief If this bit is set the Caps Lock key is enabled.
	  *
	  *  If this bit is set the Caps Lock key is enabled and the @ref
	  *  GLFW_LOCK_KEY_MODS input mode is set.
	  */
//#define MF_MOD_CAPS_LOCK       0x0010
	  /*! @brief If this bit is set the Num Lock key is enabled.
	   *
	   *  If this bit is set the Num Lock key is enabled and the @ref
	   *  GLFW_LOCK_KEY_MODS input mode is set.
	   */
//#define MF_MOD_NUM_LOCK        0x0020

	   /*! @} */