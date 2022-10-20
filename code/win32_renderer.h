/* date = October 3rd 2021 3:10 pm */

#ifndef WIN32_RENDERER_H
#define WIN32_RENDERER_H

#include <windows.h>

// NOTE(Momo): Because MS has baggage :(
#undef near
#undef far

#include "momo.h"
#include "game_renderer.h"


// NOTE(Momo): Interfaces for all win32 renderers
#define Win32_Renderer_DeclareInitFunction(Name) Renderer* Name(HWND window, u32 render_commands_size)
typedef Win32_Renderer_DeclareInitFunction(Win32_Renderer_Init_Func);

#define Win32_Renderer_DeclareFreeFunction(Name) void Name(Renderer* renderer)
typedef Win32_Renderer_DeclareFreeFunction(Win32_Renderer_Free_Func);

#define Win32_Renderer_DeclareBeginFrameFunction(Name) Mailbox* Name(Renderer* renderer, Vec2u render_wh, Rect2u render_region)
typedef Win32_Renderer_DeclareBeginFrameFunction(Win32_Renderer_BeginFrame_Func);

#define Win32_Renderer_DeclareEndFrameFunction(Name) void Name(Renderer* renderer, Mailbox* render_commands)
typedef Win32_Renderer_DeclareEndFrameFunction(Win32_Renderer_EndFrame_Func);

#define Win32_Renderer_DeclareResizeFunction(Name) void Name(Renderer* renderer, u32 width, u32 height)
typedef Win32_Renderer_DeclareResizeFunction(Win32_Renderer_Resize_Func);

#define Win32_Renderer_DeclareAddTextureFunction(Name) Renderer_Texture_Handle Name(Renderer* renderer, u32 width, u32 height, void* pixels)
typedef Win32_Renderer_DeclareAddTextureFunction(Win32_Renderer_AddTexture_Func);

#define Win32_Renderer_DeclareClearTexturesFunction(Name) void Name(Renderer* renderer)
typedef Win32_Renderer_DeclareClearTexturesFunction(Win32_Renderer_ClearTextures_Func);

struct Win32_Renderer_Functions {
	Win32_Renderer_Init_Func* init;
	Win32_Renderer_Free_Func* free;
	Win32_Renderer_BeginFrame_Func* begin_frame;
	Win32_Renderer_EndFrame_Func* end_frame;
	Win32_Renderer_AddTexture_Func* add_texture;
	Win32_Renderer_ClearTextures_Func* clear_textures;
	
	static inline const char* names[] = {
		"Renderer_Init",
		"Renderer_Free",
		"Renderer_BeginFrame",
		"Renderer_EndFrame",
		"Renderer_AddTexture",
		"Renderer_ClearTextures",
	};
};


#endif // WIN32_RENDERER_H
