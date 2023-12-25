#pragma once

// From glfw3.h

#define MF_KEY_UNKNOWN            -1

/* Printable keys */
#define MF_KEY_SPACE              32
#define MF_KEY_APOSTROPHE         39  /* ' */
#define MF_KEY_COMMA              44  /* , */
#define MF_KEY_MINUS              45  /* - */
#define MF_KEY_PERIOD             46  /* . */
#define MF_KEY_SLASH              47  /* / */
#define MF_KEY_0                  48
#define MF_KEY_1                  49
#define MF_KEY_2                  50
#define MF_KEY_3                  51
#define MF_KEY_4                  52
#define MF_KEY_5                  53
#define MF_KEY_6                  54
#define MF_KEY_7                  55
#define MF_KEY_8                  56
#define MF_KEY_9                  57
#define MF_KEY_SEMICOLON          59  /* ; */
#define MF_KEY_EQUAL              61  /* = */
#define MF_KEY_A                  65
#define MF_KEY_B                  66
#define MF_KEY_C                  67
#define MF_KEY_D                  68
#define MF_KEY_E                  69
#define MF_KEY_F                  70
#define MF_KEY_G                  71
#define MF_KEY_H                  72
#define MF_KEY_I                  73
#define MF_KEY_J                  74
#define MF_KEY_K                  75
#define MF_KEY_L                  76
#define MF_KEY_M                  77
#define MF_KEY_N                  78
#define MF_KEY_O                  79
#define MF_KEY_P                  80
#define MF_KEY_Q                  81
#define MF_KEY_R                  82
#define MF_KEY_S                  83
#define MF_KEY_T                  84
#define MF_KEY_U                  85
#define MF_KEY_V                  86
#define MF_KEY_W                  87
#define MF_KEY_X                  88
#define MF_KEY_Y                  89
#define MF_KEY_Z                  90
#define MF_KEY_LEFT_BRACKET       91  /* [ */
#define MF_KEY_BACKSLASH          92  /* \ */
#define MF_KEY_RIGHT_BRACKET      93  /* ] */
#define MF_KEY_GRAVE_ACCENT       96  /* ` */
#define MF_KEY_WORLD_1            161 /* non-US #1 */
#define MF_KEY_WORLD_2            162 /* non-US #2 */

/* Function keys */
#define MF_KEY_ESCAPE             256
#define MF_KEY_ENTER              257
#define MF_KEY_TAB                258
#define MF_KEY_BACKSPACE          259
#define MF_KEY_INSERT             260
#define MF_KEY_DELETE             261
#define MF_KEY_RIGHT              262
#define MF_KEY_LEFT               263
#define MF_KEY_DOWN               264
#define MF_KEY_UP                 265
#define MF_KEY_PAGE_UP            266
#define MF_KEY_PAGE_DOWN          267
#define MF_KEY_HOME               268
#define MF_KEY_END                269
#define MF_KEY_CAPS_LOCK          280
#define MF_KEY_SCROLL_LOCK        281
#define MF_KEY_NUM_LOCK           282
#define MF_KEY_PRINT_SCREEN       283
#define MF_KEY_PAUSE              284
#define MF_KEY_F1                 290
#define MF_KEY_F2                 291
#define MF_KEY_F3                 292
#define MF_KEY_F4                 293
#define MF_KEY_F5                 294
#define MF_KEY_F6                 295
#define MF_KEY_F7                 296
#define MF_KEY_F8                 297
#define MF_KEY_F9                 298
#define MF_KEY_F10                299
#define MF_KEY_F11                300
#define MF_KEY_F12                301
#define MF_KEY_F13                302
#define MF_KEY_F14                303
#define MF_KEY_F15                304
#define MF_KEY_F16                305
#define MF_KEY_F17                306
#define MF_KEY_F18                307
#define MF_KEY_F19                308
#define MF_KEY_F20                309
#define MF_KEY_F21                310
#define MF_KEY_F22                311
#define MF_KEY_F23                312
#define MF_KEY_F24                313
#define MF_KEY_F25                314
#define MF_KEY_KP_0               320
#define MF_KEY_KP_1               321
#define MF_KEY_KP_2               322
#define MF_KEY_KP_3               323
#define MF_KEY_KP_4               324
#define MF_KEY_KP_5               325
#define MF_KEY_KP_6               326
#define MF_KEY_KP_7               327
#define MF_KEY_KP_8               328
#define MF_KEY_KP_9               329
#define MF_KEY_KP_DECIMAL         330
#define MF_KEY_KP_DIVIDE          331
#define MF_KEY_KP_MULTIPLY        332
#define MF_KEY_KP_SUBTRACT        333
#define MF_KEY_KP_ADD             334
#define MF_KEY_KP_ENTER           335
#define MF_KEY_KP_EQUAL           336
#define MF_KEY_LEFT_SHIFT         340
#define MF_KEY_LEFT_CONTROL       341
#define MF_KEY_LEFT_ALT           342
#define MF_KEY_LEFT_SUPER         343
#define MF_KEY_RIGHT_SHIFT        344
#define MF_KEY_RIGHT_CONTROL      345
#define MF_KEY_RIGHT_ALT          346
#define MF_KEY_RIGHT_SUPER        347
#define MF_KEY_MENU               348

#define MF_KEY_LAST               MF_KEY_MENU

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
#define MF_MOD_SHIFT           0x0001
/*! @brief If this bit is set one or more Control keys were held down.
 *
 *  If this bit is set one or more Control keys were held down.
 */
#define MF_MOD_CONTROL         0x0002
   /*! @brief If this bit is set one or more Alt keys were held down.
	*
	*  If this bit is set one or more Alt keys were held down.
	*/
#define MF_MOD_ALT             0x0004
	/*! @brief If this bit is set one or more Super keys were held down.
	 *
	 *  If this bit is set one or more Super keys were held down.
	 */
#define MF_MOD_SUPER           0x0008
	 /*! @brief If this bit is set the Caps Lock key is enabled.
	  *
	  *  If this bit is set the Caps Lock key is enabled and the @ref
	  *  GLFW_LOCK_KEY_MODS input mode is set.
	  */
#define MF_MOD_CAPS_LOCK       0x0010
	  /*! @brief If this bit is set the Num Lock key is enabled.
	   *
	   *  If this bit is set the Num Lock key is enabled and the @ref
	   *  GLFW_LOCK_KEY_MODS input mode is set.
	   */
#define MF_MOD_NUM_LOCK        0x0020

	   /*! @} */