#pragma once


#include "Magnefu/Core/Platform.hpp"
#include "Magnefu/Core/Assertions.h"


namespace Magnefu
{
	enum Keys
	{

		// From glfw3.h

		 MF_KEY_UNKNOWN    =        -1,

		/* Printable keys */
		 MF_KEY_SPACE         =     32,
		 MF_KEY_APOSTROPHE    =     39,  /* ' */
		 MF_KEY_COMMA         =     44,  /* , */
		 MF_KEY_MINUS         =     45,  /* - */
		 MF_KEY_PERIOD        =     46,  /* . */
		 MF_KEY_SLASH         =     47,  /* / */
		 MF_KEY_0             =     48,
		 MF_KEY_1             =     49,
		 MF_KEY_2             =     50,
		 MF_KEY_3             =     51,
		 MF_KEY_4             =     52,
		 MF_KEY_5             =     53,
		 MF_KEY_6             =     54,
		 MF_KEY_7             =     55,
		 MF_KEY_8             =     56,
		 MF_KEY_9             =     57,
		 MF_KEY_SEMICOLON     =     59,  /* ; */
		 MF_KEY_EQUAL         =     61,  /* = */
		 MF_KEY_A             =     65,
		 MF_KEY_B             =     66,
		 MF_KEY_C             =     67,
		 MF_KEY_D             =     68,
		 MF_KEY_E             =     69,
		 MF_KEY_F             =     70,
		 MF_KEY_G             =     71,
		 MF_KEY_H             =     72,
		 MF_KEY_I             =     73,
		 MF_KEY_J             =     74,
		 MF_KEY_K             =     75,
		 MF_KEY_L             =     76,
		 MF_KEY_M             =     77,
		 MF_KEY_N             =     78,
		 MF_KEY_O             =     79,
		 MF_KEY_P             =     80,
		 MF_KEY_Q             =     81,
		 MF_KEY_R             =     82,
		 MF_KEY_S             =     83,
		 MF_KEY_T             =     84,
		 MF_KEY_U             =     85,
		 MF_KEY_V             =     86,
		 MF_KEY_W             =     87,
		 MF_KEY_X             =     88,
		 MF_KEY_Y             =     89,
		 MF_KEY_Z             =     90,
		 MF_KEY_LEFT_BRACKET  =     91,  /* [ */
		 MF_KEY_BACKSLASH     =     92,  /* \ */
		 MF_KEY_RIGHT_BRACKET =     93,  /* ] */
		 MF_KEY_GRAVE_ACCENT  =     96,  /* ` */
		 MF_KEY_WORLD_1       =     161, /* non-US #1 */
		 MF_KEY_WORLD_2       =     162, /* non-US #2 */
					  
		/*Function  keys*/
		 MF_KEY_ESCAPE        =     256,
		 MF_KEY_ENTER         =     257,
		 MF_KEY_TAB           =     258,
		 MF_KEY_BACKSPACE     =     259,
		 MF_KEY_INSERT        =     260,
		 MF_KEY_DELETE        =     261,
		 MF_KEY_RIGHT         =     262,
		 MF_KEY_LEFT          =     263,
		 MF_KEY_DOWN          =     264,
		 MF_KEY_UP            =     265,
		 MF_KEY_PAGE_UP       =     266,
		 MF_KEY_PAGE_DOWN     =     267,
		 MF_KEY_HOME          =     268,
		 MF_KEY_END           =     269,
		 MF_KEY_CAPS_LOCK     =     280,
		 MF_KEY_SCROLL_LOCK   =     281,
		 MF_KEY_NUM_LOCK      =     282,
		 MF_KEY_PRINT_SCREEN  =     283,
		 MF_KEY_PAUSE         =     284,
		 MF_KEY_F1            =     290,
		 MF_KEY_F2            =     291,
		 MF_KEY_F3            =     292,
		 MF_KEY_F4            =     293,
		 MF_KEY_F5            =     294,
		 MF_KEY_F6            =     295,
		 MF_KEY_F7            =     296,
		 MF_KEY_F8            =     297,
		 MF_KEY_F9            =     298,
		 MF_KEY_F10           =     299,
		 MF_KEY_F11           =     300,
		 MF_KEY_F12           =     301,
		 MF_KEY_F13           =     302,
		 MF_KEY_F14           =     303,
		 MF_KEY_F15           =     304,
		 MF_KEY_F16           =     305,
		 MF_KEY_F17           =     306,
		 MF_KEY_F18           =     307,
		 MF_KEY_F19           =     308,
		 MF_KEY_F20           =     309,
		 MF_KEY_F21           =     310,
		 MF_KEY_F22           =     311,
		 MF_KEY_F23           =     312,
		 MF_KEY_F24           =     313,
		 MF_KEY_F25           =     314,
		 MF_KEY_KP_0          =     320,
		 MF_KEY_KP_1          =     321,
		 MF_KEY_KP_2          =     322,
		 MF_KEY_KP_3          =     323,
		 MF_KEY_KP_4          =     324,
		 MF_KEY_KP_5          =     325,
		 MF_KEY_KP_6          =     326,
		 MF_KEY_KP_7          =     327,
		 MF_KEY_KP_8          =     328,
		 MF_KEY_KP_9          =     329,
		 MF_KEY_KP_DECIMAL    =     330,
		 MF_KEY_KP_DIVIDE     =     331,
		 MF_KEY_KP_MULTIPLY   =     332,
		 MF_KEY_KP_SUBTRACT   =     333,
		 MF_KEY_KP_ADD        =     334,
		 MF_KEY_KP_ENTER      =     335,
		 MF_KEY_KP_EQUAL      =     336,
		 MF_KEY_LEFT_SHIFT    =     340,
		 MF_KEY_LEFT_CONTROL  =     341,
		 MF_KEY_LEFT_ALT      =     342,
		 MF_KEY_LEFT_SUPER    =     343,
		 MF_KEY_RIGHT_SHIFT   =     344,
		 MF_KEY_RIGHT_CONTROL =     345,
		 MF_KEY_RIGHT_ALT     =     346,
		 MF_KEY_RIGHT_SUPER   =     347,
		 MF_KEY_MENU          =     348,

		 MF_KEY_LAST          =     MF_KEY_MENU,
		 MF_KEY_COUNT
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

	cstring* KeyNames();
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