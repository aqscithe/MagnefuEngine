#pragma once

// -- Application Includes ---------//
#include "ImGuiDefs.h"
#include "Magnefu/Application/Layer.h"
#include "Magnefu/Application/Events/KeyEvent.h"
#include "Magnefu/Application/Events/MouseEvent.h"

// -- Graphics Includes --------------------- //


// -- Core Includes ---------------------------------- //
#include "Magnefu/Core/Service.hpp"

// -- Other Includes ------------------ //
#include <stdint.h>



namespace Magnefu
{
	// -- Forward Declarations -------------------- //
	//struct ImDrawData;

	struct GraphicsContext;
	struct CommandBuffer;
	struct TextureHandle;


	//TODO: cHANGE THESE COLORS
	enum ImGuiStyles 
	{
		Default = 0,
		GreenBlue,
		DarkRed,
		DarkGold,
		DarkClassic
	}; // enum ImGuiStyles


	struct ImGuiServiceConfiguration 
	{

		GraphicsContext* gpu;
		void* window_handle;

	}; // struct ImGuiServiceConfiguration



	struct  ImGuiService : public Service
	{

	public:
		ImGuiService();
		~ImGuiService();

		MF_DECLARE_SERVICE(ImGuiService);

		void                            Init(void* configuration) override;
		void                            Shutdown() override;

		void                            BeginFrame();
		void                            Render(CommandBuffer& commands);

		// Removes the Texture from the Cache and destroy the associated Descriptor Set.
		void                            RemoveCachedTexture(TextureHandle& texture);

		void                            SetStyle(ImGuiStyles style);

		


	public:
		GraphicsContext* gpu;

		static constexpr cstring        k_name = "Magnefu_ImGui_Service";
	};




	// File Dialog /////////////////////////////////////////////////////////

	/*bool                                imgui_file_dialog_open( const char* button_name, const char* path, const char* extension );
	const char*                         imgui_file_dialog_get_filename();

	bool                                imgui_path_dialog_open( const char* button_name, const char* path );
	const char*                         imgui_path_dialog_get_path();*/

	// Application Log /////////////////////////////////////////////////////

	void                                imgui_log_init();
	void                                imgui_log_shutdown();

	void                                imgui_log_draw();

	// FPS graph ///////////////////////////////////////////////////
	void                                imgui_fps_init();
	void                                imgui_fps_shutdown();
	void                                imgui_fps_add(f32 dt);
	void                                imgui_fps_draw();
}

