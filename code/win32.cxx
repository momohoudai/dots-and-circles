#include <Shlobj.h>

#include <windows.h>
#include <windowsx.h>
#include <ShellScalingAPI.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <imm.h>



// NOTE(Momo): Because MS has baggage :(
#undef near
#undef far

#include "momo.h"

#include "game_config.h"
#include "game_renderer.h"
#include "game_platform.h"

#include "win32_renderer.h"


//~ NOTE(Momo): Globals

#if INTERNAL
#define WIN32_RECORD_STATE_FILE "record_state"
#define WIN32_RECORD_INPUT_FILE "record_input"
#define WIN32_SAVE_STATE_FILE "game_state"
#endif // INTERNAL

#define WIN32_APPDATA_DIRECTORY "momodevelop\\dnc\\"
#define WIN32_LOG_FILE "log"

//~ NOTE(Momo): Common
static inline u32
Win32_DetermineIdealRefreshRate(HWND window, u32 default_refresh_rate) {
  // Do we want to cap this?
  HDC dc = GetDC(window);
  defer { ReleaseDC(window, dc); };
  
  u32 refresh_rate = default_refresh_rate;
  {
    s32 display_refresh_rate = GetDeviceCaps(dc, VREFRESH);
    // It is possible for the refresh rate to be 0 or less
    // because of something called 'adaptive vsync'
    if (display_refresh_rate > 1) {
      refresh_rate = display_refresh_rate;
    }
  }
  return refresh_rate;
  
}

static inline LARGE_INTEGER
Win32_FileTimeToLargeInt(FILETIME file_time) {
  LARGE_INTEGER ret = {};
  ret.LowPart = file_time.dwLowDateTime;
  ret.HighPart = file_time.dwHighDateTime;
  
  return ret;
}

static inline LONG
Width(RECT value) {
  return value.right - value.left;
}

static inline LONG
Height(RECT value) {
  return value.bottom - value.top;
}


static inline LARGE_INTEGER 
Win32_GetFileLastWriteTime(const char* filename) {
  WIN32_FILE_ATTRIBUTE_DATA data;
  FILETIME last_write_time = {};
  
  if(GetFileAttributesEx(filename, GetFileExInfoStandard, &data)) {
    last_write_time = data.ftLastWriteTime;
  }
  return Win32_FileTimeToLargeInt(last_write_time); 
}


static inline void*
Win32_AllocateMemory(umi memory_size) {
  return VirtualAllocEx(GetCurrentProcess(),
                        0, 
                        memory_size,
                        MEM_RESERVE | MEM_COMMIT, 
                        PAGE_READWRITE);
  
}

static inline void
Win32_FreeMemory(void* memory) {
  if(memory) {
    VirtualFreeEx(GetCurrentProcess(), 
                  memory,    
                  0, 
                  MEM_RELEASE); 
  }
}

static inline LARGE_INTEGER
Win32_GetPerformanceCounter(void) {
  LARGE_INTEGER result;
  QueryPerformanceCounter(&result);
  return result;
}


//~ NOTE(Momo): Logging

#include "win32_logger.h"
Win32_Logger* g_logger = {};

static inline
Platform_DeclareLogFunction(Win32_Log) {
  u8 buffer[256];
  StringBuffer sb = {};
	sb.data = buffer;
	sb.capacity = ArrayCount(buffer);
	
	
  va_list args;
  va_start(args, format);
  
  //stbsp_vsprintf(buffer, format, valist);
	StringBuffer_FormatList(&sb, String_Create(format), args);
  
  Win32_Logger_Write(g_logger, sb.str);
  va_end(args);
}


//~ NOTE(Momo): Start of implementations
#include "win32_game_memory.h"
#include "win32_record_playback.h"
#include "win32_imported_code.h"
#include "win32_audio.h"
#include "win32_renderer.h"

struct Win32_State {
  b8 is_running;
  
  void* platform_memory_block;
  u32 platform_memory_block_size;	
  u32 performance_frequency;
	
	u8 appdata_path_buffer[MAX_PATH];
	StringBuffer appdata_path; // for internal use, this is root folder	
	
  // Handle pool
  HANDLE handles[4];
  u32 handle_free_list[ArrayCount(handles)];
  u32 handle_free_count;
	
	
  Renderer* renderer;
	Win32_ImportedCode<Win32_Renderer_Functions>* renderer_code;
	
#if INTERNAL
	// NOTE(Momo): We only have one asset file so doing this is okay for now
	LARGE_INTEGER prev_asset_write_time;
#endif // INTERNAL
	
};
Win32_State* g_state = {};


//~ NOTE(Momo): Win32 state related
static inline f32
Win32_GetSecondsElapsed(Win32_State* state,
                        LARGE_INTEGER start, 
                        LARGE_INTEGER end) 
{
  return (f32(end.QuadPart - start.QuadPart)) / state->performance_frequency; 
}


static inline b8
Win32_Init(Win32_State* state) {
	// NOTE(Momo): Initialize appdata path and folders
	{
		char appdata_root_path[MAX_PATH] = {0};
		
#if INTERNAL
		appdata_root_path[0] = '.';
		
#else
		if (!SHGetSpecialFolderPathA(NULL, 
                                 appdata_root_path,
                                 CSIDL_APPDATA,
                                 0))
		{
			return false;
		}
#endif // INTERNAL
		
		
		if (!StringBuffer_Init(&state->appdata_path, 
                           state->appdata_path_buffer, 
                           sizeof(state->appdata_path_buffer)))
		{
			return false;
		}
		StringBuffer_PushSistr(&state->appdata_path, appdata_root_path);
		StringBuffer_PushU8(&state->appdata_path, '\\');
		StringBuffer_PushSistr(&state->appdata_path, WIN32_APPDATA_DIRECTORY);
		
		// NOTE(Momo): Add null terminating value
		if (!StringBuffer_PushU8(&state->appdata_path, '\0')) {
			return false;
		}
		
#if !INTERNAL
		int create_dir_result = SHCreateDirectoryExA(NULL, 
                                                 (char*)state->appdata_path.str.data, 
                                                 NULL);
		if (create_dir_result != ERROR_SUCCESS &&
        create_dir_result != ERROR_FILE_EXISTS &&
        create_dir_result != ERROR_ALREADY_EXISTS) 
		{
			return false;
		}
#endif
		
		// NOTE(Momo): Remove null terminating value
		StringBuffer_Pop(&state->appdata_path);
		
		
	}		
	
	// NOTE(Momo): Performance Frequency 
	LARGE_INTEGER perf_count_freq;
	QueryPerformanceFrequency(&perf_count_freq);
	state->performance_frequency = u32(perf_count_freq.QuadPart);
	
	// NOTE(Momo): initialize file handle store
	//Globalfile_handles = CreatePool<HANDLE>(&g_state->Arena, 8);
	for (u32 i = 0; i < ArrayCount(state->handles); ++i) {
		state->handle_free_list[i] = i;
	}
	state->handle_free_count = ArrayCount(state->handles);
	
	state->is_running = true;
	return true;
	
	
	
}


static inline void
Win32_Free(Win32_State* state) {
	Win32_FreeMemory(state); 
	
}

static inline void
Win32_SwapBuffers(HWND window) {
	HDC DeviceContext = GetDC(window); 
	defer { ReleaseDC(window, DeviceContext); };
	SwapBuffers(DeviceContext);
}





static inline Vec2u
Win32_GetDesktopDimensions() {
	Vec2u ret = {};
	
	auto active_window = GetActiveWindow();
	HMONITOR monitor = MonitorFromWindow(active_window, MONITOR_DEFAULTTONEAREST);
	MONITORINFOEX monitor_info;
	monitor_info.cbSize = sizeof(monitor_info);
	GetMonitorInfo(monitor, &monitor_info); 
	
	ret.w = u32(monitor_info.rcWork.right - monitor_info.rcWork.left);
	ret.h = u32(monitor_info.rcWork.bottom - monitor_info.rcWork.top);
	return ret;
}



static inline Vec2u
Win32_GetWindowDimensions(HWND window) {
	RECT rect = {};
	GetWindowRect(window, &rect);
	return { u16(rect.right - rect.left), u16(rect.bottom - rect.top) };
	
}

static inline Vec2u
Win32_GetClientDimensions(HWND window) {
	RECT rect = {};
	GetClientRect(window, &rect);
	return { u32(rect.right - rect.left), u32(rect.bottom - rect.top) };
	
}


static inline void
Win32_ProcessMessages(HWND window, 
                      Win32_State* state,
                      Platform_Input* input,
                      Win32_GameMemory* game_memory,
                      Renderer* renderer
#if INTERNAL
                      ,
                      Win32_InputRecorder* input_recorder,
                      Win32_InputPlaybacker * input_playbacker
#endif // INTERNAL
                      )
{
	MSG msg = {};
	while(PeekMessage(&msg, window, 0, 0, PM_REMOVE)) {
		switch(msg.message) {
			case WM_QUIT:
			case WM_CLOSE: {
				state->is_running = false;
			} break;
#if INTERNAL
			case WM_CHAR: {
				Platform_Input_PushChar(input, ((u8)msg.wParam));
			} break;
#endif // INTERNAL
			case WM_LBUTTONUP:
			case WM_LBUTTONDOWN: {
				u32 code = (u32)msg.wParam;
				b8 is_down = msg.message == WM_LBUTTONDOWN;
				input->button_switch.now = is_down;
			} break;
			
			case WM_RBUTTONUP:
			case WM_RBUTTONDOWN:{
				u32 code = (u32)msg.wParam;
				b8 is_down = msg.message == WM_RBUTTONDOWN;
				input->button_pause.now = is_down;
			} break;
#if INTERNAL					
			case WM_SYSKEYDOWN:
			case WM_SYSKEYUP:
			case WM_KEYDOWN:
			case WM_KEYUP: {
				u32 KeyCode = (u32)msg.wParam;
				b8 is_down = msg.message == WM_KEYDOWN;
				switch(KeyCode) {
					case VK_RETURN:{
						input->button_confirm.now = is_down;
					} break;
					case VK_F1:{
						input->button_console.now = is_down;
					} break;
					case VK_F2:{
						input->button_inspector.now = is_down;
					} break;
					
					case VK_F3:{
						if (msg.message == WM_KEYDOWN) {
							if (Win32_GameMemory_Save(game_memory, WIN32_SAVE_STATE_FILE)) {
								Win32_Log("[Win32] Save game memory success...\n");
							}
							else {
								Win32_Log("[Win32] Save game memory failed...\n");
							}
							
						}
					} break;
					case VK_F4:{
						if (msg.message == WM_KEYDOWN) {
							if (Win32_GameMemory_Load(game_memory, WIN32_SAVE_STATE_FILE)) {
								Win32_Log("[Win32] Load game memory success...\n");
							}
							else {
								Win32_Log("[Win32] Load game memory failed...\n");
							}
						}
					} break;
					case VK_F5:{
						if (msg.message == WM_KEYDOWN) {
							if(input_recorder->is_recording_input) {
								if (Win32_InputRecorder_End(input_recorder)) {
									Win32_Log("[Win32] End input recorder failed...\n");
								}
								else {
									Win32_Log("[Win32] End input recorder success\n");
								}
							}
							else {
								if (Win32_GameMemory_Save(game_memory, WIN32_RECORD_STATE_FILE) &&
                    Win32_InputRecorder_Begin(input_recorder, WIN32_RECORD_INPUT_FILE))
								{
									Win32_Log("[Win32] Start input recorder success\n");
								}
								else {
									Win32_Log("[Win32] Start input recorder failed\n");
								}
								
							}
						}
						
					} break;
					case VK_F6:{
						
						if (msg.message == WM_KEYDOWN) {
							if(input_playbacker->is_playback_input) {
								if (Win32_InputPlaybacker_End(input_playbacker)) {
									Win32_Log("[Win32] Stop input playback success\n");
								}
								else {
									Win32_Log("[Win32] Stop input playback failed\n");
								}
							}
							else {
								if (Win32_GameMemory_Load(game_memory, WIN32_RECORD_STATE_FILE) &&
                    Win32_InputPlaybacker_Begin(input_playbacker, WIN32_RECORD_INPUT_FILE)) 
								{
									Win32_Log("[Win32] Start input playback success\n");
								}
								else {
									Win32_Log("[Win32] Start input playback failed\n");
								}
								
							}
						}
						
					} break;
					
					case VK_F11:{
						input->button_speed_down.now = is_down;
					} break;
					case VK_F12: {
						input->button_speed_up.now = is_down;
					} break;
					case VK_BACK:{
						input->button_back.now = is_down;
					} break;	
				} 
				TranslateMessage(&msg);
				
			} break;
			
#else // INTERNAL
			case WM_SYSKEYDOWN:
			case WM_SYSKEYUP:
			case WM_KEYDOWN:
			case WM_KEYUP: {
				
				// NOTE(Momo): Ignore all keys.
				// We have to do this especially for function keys,
				// most notablebly F10, whose default behaviour is to
				// open window menu, and thereby pausing the whole applications.
				
			} break;
#endif // INTERNAL
			default: 
			{
				//Win32_Log("[Win32::ProcessMessages] %d\n", msg.message);
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			} break;
		}
	}
}

LRESULT CALLBACK
Win32_WindowCallback(HWND window, 
                     UINT message, 
                     WPARAM w_param,
                     LPARAM l_param) 
{
	LRESULT result = 0;
	switch(message) {
		case WM_CLOSE: {
			g_state->is_running = false;
		} break;
		case WM_DESTROY: {
			g_state->is_running = false;
		} break;
		default: {
			//TODO: Log message?
			result = DefWindowProcA(window, message, w_param, l_param);
		};   
	}
	return result;
}


static inline HWND 
Win32_CreateWindow(HINSTANCE instance,
                   f32 aspect_ratio,
                   const char* title,
                   const char* icon,
                   u32 icon_width,
                   u32 icon_height) 
{
	WNDCLASSA window_class = {};
	window_class.style = CS_HREDRAW | CS_VREDRAW;
	window_class.lpfnWndProc = Win32_WindowCallback;
	window_class.hInstance = instance;
	window_class.hCursor = LoadCursor(0, IDC_ARROW);
	window_class.lpszClassName = "MainWindowClass";
	window_class.hIcon = (HICON)LoadImageA(NULL, 
                                         icon,
                                         IMAGE_ICON, 
                                         icon_width, 
                                         icon_height,
                                         LR_LOADFROMFILE);
	
	if(!RegisterClassA(&window_class)) {
		Win32_Log("[Win32::window] Failed to create class\n");
		return NULL;
	}
	
  // calculate the best window size. 
  // 
  TITLEBARINFOEX title_bar_info;
  title_bar_info.cbSize = sizeof(title_bar_info);
  
  Vec2u desktop_dimensions = Win32_GetDesktopDimensions();
  u32 ideal_window_width = desktop_dimensions.w / 10 * 9;
  u32 ideal_window_height = desktop_dimensions.h / 10 * 9;
  u32 window_width = 0;
  u32 window_height = 0;
  
  if (ideal_window_width < ideal_window_height) {
    window_width = ideal_window_width;
    window_height = (u32)((f32)window_width * 1.f/aspect_ratio);
  }
  else {
    window_height = ideal_window_height;
    window_width = (u32)((f32)window_height * aspect_ratio);
  }
  
  HWND window = {};
	RECT window_rect = {};
  
	window_rect.left = desktop_dimensions.w / 2 - window_width / 2;
	window_rect.right = desktop_dimensions.w / 2 + window_width / 2;
	window_rect.top = desktop_dimensions.h / 2 - window_height / 2;
	window_rect.bottom = desktop_dimensions.h / 2 + window_height / 2;
	
	DWORD style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
	AdjustWindowRectEx(&window_rect,
                     style,
                     FALSE,
                     0);
	
	
	window = CreateWindowExA(0,
                           window_class.lpszClassName,
                           title,
                           style,
                           window_rect.left,
                           window_rect.top,
                           Width(window_rect),
                           Height(window_rect),
                           0,
                           0,
                           instance,
                           0);
	
	
	if (!window) {
		Win32_Log("[Win32::window] Failed to create window\n");
		return NULL;
	}
	
	Win32_Log("[Win32::window] window created successfully\n");
	Vec2u window_wh = Win32_GetWindowDimensions(window);
	Vec2u client_wh = Win32_GetClientDimensions(window);
	Win32_Log("[Win32::window] client: %d x %d\n", client_wh.w, client_wh.h);
	Win32_Log("[Win32::window] window: %d x %d\n", window_wh.w, window_wh.h);
	return window;
	
}

// Platform Functions ////////////////////////////////////////////////////
enum Win32_File_Error_Type {
	WIN32_FILE_ERROR_NONE,
	WIN32_FILE_ERROR_NO_HANDLERS,
	WIN32_FILE_ERROR_CANNOT_OPEN,   
	WIN32_FILE_ERROR_ALREADY_CLOSED,
	WIN32_FILE_ERROR_READ_FILE_FAILED,
	WIN32_FILE_ERROR_WRITE_FILE_FAILED,
	WIN32_FILE_ERROR_PATH_TOO_LONG,
};


static inline
Platform_DeclareGetPerformanceCounterFunction(Win32_GetPerformanceCounterU64) {
	return (u64)Win32_GetPerformanceCounter().QuadPart;
}

static inline 
Platform_DeclareOpenFileFunction(Win32_OpenFile) {
	Platform_File_Handle ret = {}; 
	
	// Check if there are free handlers to go around
	if (g_state->handle_free_count == 0) {
		ret.error = WIN32_FILE_ERROR_NO_HANDLERS;
		return ret;
	}    
	
	u8 buffer[MAX_PATH];
	StringBuffer sb = {};
	
	HANDLE win32_handle = INVALID_HANDLE_VALUE;
	
	switch(location) {
		case Platform_File_Location_User: {
			if (StringBuffer_Init(&sb, buffer, MAX_PATH)) {
				StringBuffer_PushString(&sb, g_state->appdata_path.str);
				StringBuffer_PushU8(&sb, '\\');
				StringBuffer_PushSistr(&sb, file);
				if (!StringBuffer_PushU8(&sb, '\0')) {
					ret.error = WIN32_FILE_ERROR_PATH_TOO_LONG;
					return ret;
				}
			}
		} break;
		
		case Platform_File_Location_Game: {
			if (StringBuffer_Init(&sb, buffer, MAX_PATH)) {
				StringBuffer_PushU8(&sb, '.');
				StringBuffer_PushU8(&sb, '\\');
				StringBuffer_PushSistr(&sb, file);
				if (!StringBuffer_PushU8(&sb, '\0')) {
					ret.error = WIN32_FILE_ERROR_PATH_TOO_LONG;
					return ret;
				}
			}
		} break;
	}
	
	switch (open_mode) {
		case Platform_File_Open_Mode_Read: {
			win32_handle = CreateFileA((char*)sb.data, 
                                 GENERIC_READ, 
                                 FILE_SHARE_READ,
                                 0,
                                 OPEN_EXISTING,
                                 0,
                                 0);
			
		} break;
		case Platform_File_Open_Mode_Overwrite: {
			win32_handle = CreateFileA((char*)sb.data, 
                                 GENERIC_READ | GENERIC_WRITE, 
                                 FILE_SHARE_READ,
                                 0,
                                 CREATE_ALWAYS,
                                 0,
                                 0);				
		} break;
	}
	
	
	
	if(win32_handle == INVALID_HANDLE_VALUE) {
		ret.error = WIN32_FILE_ERROR_CANNOT_OPEN;
		return ret;
	} 
	
	u32 free_slot_id = g_state->handle_free_list[g_state->handle_free_count-1];
	g_state->handles[free_slot_id] = win32_handle;
	--g_state->handle_free_count;
	ret.id = free_slot_id;
	
	return ret; 
}

static inline 
Platform_DeclareLogFileErrorFunction(Win32_LogFileError) {
	switch(handle->error) {
		case WIN32_FILE_ERROR_NONE: {
			Win32_Log("[Win32::File] There is no file error\n");
		} break;
		case WIN32_FILE_ERROR_NO_HANDLERS: {
			Win32_Log("[Win32::File] There is not enough handlers\n");
		} break;
		case WIN32_FILE_ERROR_CANNOT_OPEN:{
			Win32_Log("[Win32::File] Cannot open file\n");
		} break;
		case WIN32_FILE_ERROR_ALREADY_CLOSED:{
			Win32_Log("[Win32::File] File is already closed\n");
		} break;
		case WIN32_FILE_ERROR_READ_FILE_FAILED: {
			Win32_Log("[Win32::File] File read failed\n");
		} break;
		default: {
			Win32_Log("[Win32::File] Undefined error!\n");
		};
	}
}

static inline
Platform_DeclareCloseFileFunction(Win32_CloseFile) {
	Assert(handle->id < ArrayCount(g_state->handles));
	HANDLE win32_handle = g_state->handles[handle->id];
	if (win32_handle != INVALID_HANDLE_VALUE) {
		CloseHandle(win32_handle); 
	}
	g_state->handle_free_list[g_state->handle_free_count++] = handle->id;
	Assert(g_state->handle_free_count <= ArrayCount(g_state->handles));
}
static inline
Platform_DeclareAddTextureFunction(Win32_AddTexture) {
	return g_state->renderer_code->add_texture(g_state->renderer, width, height, pixels);
}

static inline 
Platform_DeclareClearTexturesFunction(Win32_ClearTextures) {
	//return Opengl_ClearTextures(g_state->opengl);
	return g_state->renderer_code->clear_textures(g_state->renderer);
}

static inline 
Platform_DeclareReadFileFunction(Win32_ReadFile) {
	if (handle->error) {
		return;
	}
	Assert(handle->id < ArrayCount(g_state->handles));
	
	HANDLE win32_handle = g_state->handles[handle->id];
	OVERLAPPED overlapped = {};
	overlapped.Offset = (u32)((offset >> 0) & 0xFFFFFFFF);
	overlapped.OffsetHigh = (u32)((offset >> 32) & 0xFFFFFFFF);
	
	u32 file_size_32 = (u32)size;
	DWORD bytes_read;
	if(ReadFile(win32_handle, dest, file_size_32, &bytes_read, &overlapped) &&
	   file_size_32 == bytes_read) 
	{
		// success;
	}
	else {
		handle->error = WIN32_FILE_ERROR_READ_FILE_FAILED; 
	}
}

static inline 
Platform_DeclareWriteFileFunction(Win32_WriteFile) {
	if (handle->error) {
		return;
	}
	Assert(handle->id < ArrayCount(g_state->handles));
	
	HANDLE win32_handle = g_state->handles[handle->id];
	OVERLAPPED overlapped = {};
	overlapped.Offset = (u32)((offset >> 0) & 0xFFFFFFFF);
	overlapped.OffsetHigh = (u32)((offset >> 32) & 0xFFFFFFFF);
	
	u32 file_size_32 = (u32)size;
	DWORD bytes_wrote;
	if(WriteFile(win32_handle, src, file_size_32, &bytes_wrote, &overlapped) &&
	   file_size_32 == bytes_wrote) 
	{
		// success
	}
	else {
		handle->error = WIN32_FILE_ERROR_WRITE_FILE_FAILED; 
	}
}

static inline 
Platform_DeclareHideCursorFunction(Win32_HideCursor) {
	while(ShowCursor(FALSE) >= 0);
}

static inline
Platform_DeclareShowCursorFunction(Win32_ShowCursor) {
	while(ShowCursor(TRUE) < 0);
}

static inline
Platform_DeclareGetFileSizeFunction(Win32_GetFileSize) 
{
	HANDLE file_handle = CreateFileA(path, 
                                   GENERIC_READ, 
                                   FILE_SHARE_READ,
                                   0,
                                   OPEN_EXISTING,
                                   0,
                                   0);
	defer { CloseHandle(file_handle); };
	
	if(file_handle == INVALID_HANDLE_VALUE) {
		Win32_Log("[Win32::GetFileSize] Cannot open file: %s\n", path);
		return 0;
	} else {
		LARGE_INTEGER file_size;
		if (!GetFileSizeEx(file_handle, &file_size)) {
			Win32_Log("[Win32::GetFileSize] Problems getting file size: %s\n", path);
			return 0;
		}
		
		return (u32)file_size.QuadPart;
	}
}

#if INTERNAL
static inline 
Platform_DeclareGetFileModifiedTimeFunction(Win32_GetFileModifiedTime) {
	// NOTE(Momo): Internal use only. We don't care about %APPDATA% files
	return Win32_GetFileLastWriteTime(filename).QuadPart;
}
#endif // INTERNAL

static inline Platform_API
Win32_Platform_API_Init() {
	Platform_API platform_api = {};
	
	platform_api.log = Win32_Log;
	platform_api.read_file = Win32_ReadFile;
	platform_api.write_file = Win32_WriteFile;
	platform_api.get_file_size = Win32_GetFileSize;
	platform_api.clear_textures = Win32_ClearTextures;
	platform_api.add_texture = Win32_AddTexture;
	//platform_api.open_asset_file = win32_open_asset_file;
	//platform_api.open_save_file = win32_open_save_file;
	platform_api.open_file = Win32_OpenFile;
	platform_api.close_file = Win32_CloseFile;
	platform_api.log_file_error = Win32_LogFileError;
	platform_api.show_cursor = Win32_ShowCursor;
	platform_api.hide_cursor = Win32_HideCursor;
	platform_api.get_performance_counter = Win32_GetPerformanceCounterU64;
	
	
#if INTERNAL
	platform_api.get_file_modified_time = Win32_GetFileModifiedTime;
#endif // INTERNAL
	
	
	return platform_api;
}

static inline Rect2u 
Win32_CalcRenderRegion(u32 window_w, 
                       u32 window_h, 
                       u32 render_w, 
                       u32 render_h) 
{
	if ( render_w == 0 || render_h == 0 || window_w == 0 || window_h == 0) {
		return {};
	}
	Rect2u ret = {};
	
	f32 optimal_window_w = (f32)window_h * ((f32)render_w / (f32)render_h);
	f32 optimal_window_h = (f32)window_w * ((f32)render_h / (f32)render_w);
	
	if (optimal_window_w > (f32)window_w) {
		// NOTE(Momo): width has priority - top and bottom bars
		ret.min.x = 0;
		ret.max.x = window_w;
		
		f32 empty_height = (f32)window_h - optimal_window_h;
		
		ret.min.y = (u32)(empty_height * 0.5f);
		ret.max.y = ret.min.y + (u32)optimal_window_h;
	}
	else {
		// NOTE(Momo): height has priority - left and right bars
		ret.min.y = 0;
		ret.max.y = window_h;
		
		
		f32 empty_width = (f32)window_w - optimal_window_w;
		
		ret.min.x = (u32)(empty_width * 0.5f);
		ret.max.x = ret.min.x + (u32)optimal_window_w;
	}
	
	return ret;
}


int CALLBACK
WinMain(HINSTANCE instance,
        HINSTANCE prev_instance,
        LPSTR command_line,
        int show_code)
{
	Win32_State state = {};
	if (!Win32_Init(&state)) {
		return 1;
	}
	g_state = &state;
	
	
	Win32_Logger logger = {};
	{
#if INTERNAL
		if (!Win32_Logger_Init(&logger, WIN32_LOG_FILE)) {
			return 1;
		}
#else //if !INTERNAL
		u8 buffer[MAX_PATH];
		StringBuffer sb = {};
		if (!StringBuffer_Init(&sb, buffer, MAX_PATH)){			
			return 1;
		}
		StringBuffer_PushString(&sb, state.appdata_path.str);
		StringBuffer_PushSistr(&sb, WIN32_LOG_FILE);
		if(!StringBuffer_PushU8(&sb, '\0')) {
			return 1;
		}
		
		if (!Win32_Logger_Init(&logger, (char*)sb.str.data)) {
			return 1;
		}
#endif // INTERNAL	
		
	}
	defer { Win32_Logger_Free(&logger); };
	g_logger = &logger; 
	
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	ImmDisableIME((DWORD)-1);
	
  
	HWND window = Win32_CreateWindow(instance, 
                                   (f32)GAME_DESIGN_WIDTH/(f32)GAME_DESIGN_HEIGHT,
                                   GAME_TITLE, 
                                   "window.ico", 
                                   255, 
                                   255);
	if (!window) { 
		Win32_Log("[Win32::Main] Cannot create window");
		return 1; 
	}
	
	u32 refresh_rate = Win32_DetermineIdealRefreshRate(window, 60); 
	f32 target_secs_per_frame = 1.f / refresh_rate; 
	Win32_Log("[Win32::Main] Target Secs Per Frame: %.2f\n", target_secs_per_frame);
	Win32_Log("[Win32::Main] Monitor Refresh Rate: %d Hz\n", refresh_rate);
	
	//- NOTE(Momo): Load the game code DLL
	Win32_ImportedCode<Game_Functions> game_code = {};
	Win32_ImportedCode_Init(&game_code, 
                          "game.dll", 
#if INTERNAL
                          "tmp_game.dll",
#endif // INTERNAL
                          "game_lock");
	
	if(!Win32_ImportedCode_Load(&game_code)) {
		Win32_Log("[Win32::Main] Failed to load game code\n");
		return 1;
	}
	Win32_Log("[Win32::Main] Game code loaded!\n");
	defer { Win32_ImportedCode_Unload(&game_code); };
	
	
	//- NOTE(Momo): Initialize renderer
	// TODO: refactor with gamecode?
	Win32_ImportedCode<Win32_Renderer_Functions> renderer_code = {};
	Win32_ImportedCode_Init(&renderer_code,
                          "renderer.dll",
#if INTERNAL
                          "tmp_renderer.dll",
#endif // INTERNAL
                          "renderer_lock");
	if(!Win32_ImportedCode_Load(&renderer_code)) {
		Win32_Log("[Win32::Main] Failed to load renderer code\n");
		return 1;
	}
	
	Win32_Log("[Win32::Main] Renderer code loaded!\n");
	defer { Win32_ImportedCode_Unload(&renderer_code); };
	state.renderer_code = &renderer_code;
	
	//- NOTE(Momo): Initialize game input
	Platform_Input game_input = {};
#if INTERNAL
	if(!Platform_Input_Init(&game_input)) {
		Win32_Log("[Win32::Main] Cannot initialize input");
		return 1;
	}
#endif // INTERNAL
	
	
	//- NOTE(Momo): Initialize platform api
	Platform_API platform_api = Win32_Platform_API_Init();
	
	//- NOTE(Momo): Initialize audio
	Win32_Audio audio = {};
	if(!Win32_Audio_Init(&audio,
                       GAME_AUDIO_SAMPLES_PER_SECOND,
                       GAME_AUDIO_BITS_PER_SAMPLE,
                       GAME_AUDIO_CHANNELS,
                       GAME_AUDIO_LATENCY_FRAMES,
                       refresh_rate)) 
	{
		Win32_Log("[Win32::Main] Cannot initialize audio\n");
		return 1;
	}
	defer { Win32_Audio_Free(&audio); }; 
	Win32_Log("[Win32::Main] Audio initialized!\n");
	
	//- NOTE(Momo): Initialize game memory
	Win32_GameMemory game_memory = {};
	
	if (!Win32_GameMemory_Init(&game_memory, GAME_MEMORY_SIZE)) 
	{
		Win32_Log("[Win32::Main] Cannot initialize game memory\n");
		return 1;
	}
	defer { Win32_GameMemory_Free(&game_memory); };
	Win32_Log("[Win32::Main] Game memory initialized!\n");
	
	//- NOTE(Momo): Initialize record and playback
#if INTERNAL
	Win32_InputRecorder input_recorder = {};
	Win32_InputPlaybacker input_playbacker = {};
#endif // INTERNAL
	
	//- NOTE(Momo): Initialize Renderer
	Renderer* renderer = renderer_code.init(window, GAME_RENDER_COMMAND_SIZE);
	if (!renderer) {
		Win32_Log("[Win32::Main] Failed to initialize renderer");
		return 1;
	}
	defer { renderer_code.free(renderer); };
	Win32_Log("[Win32::Main] Renderer initialized!\n");
	state.renderer = renderer;
	
	//- NOTE(Momo): Set sleep granularity to 1ms
	b8 is_sleep_granular = timeBeginPeriod(1) == TIMERR_NOERROR;
	
	
	//- NOTE(Momo): Game Loop
	LARGE_INTEGER last_count = Win32_GetPerformanceCounter(); 
	
#if INTERNAL
	u32 code_reload_count = 0;
#endif // INTERNAL
	
	
	while (state.is_running) {
		Vec2u client_dims = Win32_GetClientDimensions(window);
		Vec2u design_dims = Vec2u { GAME_DESIGN_WIDTH, GAME_DESIGN_HEIGHT };
		Rect2u render_region = Win32_CalcRenderRegion(client_dims.w, 
                                                  client_dims.h,
                                                  design_dims.w,
                                                  design_dims.h);
		
		//- NOTE(Momo): Input processing always come first
		Platform_Input_Update(&game_input);
		
		//- NOTE(Momo): Process window messages only after input is processed
		Win32_ProcessMessages(window, 
                          &state,
                          &game_input,
                          &game_memory,
                          renderer				  
#if INTERNAL
                          ,
                          &input_recorder,
                          &input_playbacker
#endif // INTERNAL
                          );
		
#if INTERNAL
		// NOTE(Momo): Hot reloading of code
		if (Win32_ImportedCode_IsOutdated(&game_code)) {
			Win32_ImportedCode_Unload(&game_code);
			
			Win32_Log("[Win32::Main] Reloading game code! Attempt #%d\n", code_reload_count);
			if (code_reload_count++ < 256) {
				if (Win32_ImportedCode_Load(&game_code)) {
					code_reload_count = 0;
					// IsOutdated should be false now.
				}
				else {
					Win32_Log("[Win32::Main] Failed to reload game code!\n");
					return 1;
				}
			}
			
		}
		
#endif // INTERNAL
		
		
#if INTERNAL
		// NOTE(Momo): Recording/Playback input
		if (input_recorder.is_recording_input) {
			Win32_InputRecorder_Update(&input_recorder, &game_input);
		}
		if (input_playbacker.is_playback_input) {
			// NOTE(Momo): This will actually modify game_input
			if (Win32_InputPlaybacker_Update(&input_playbacker, &game_input)) {
				Win32_InputPlaybacker_End(&input_playbacker);
				Win32_GameMemory_Load(&game_memory, WIN32_RECORD_STATE_FILE);
				Win32_InputPlaybacker_Begin(&input_playbacker,
                                    WIN32_RECORD_INPUT_FILE);
			}
		}
#endif // INTERNAL
		
		
		//- NOTE(Momo): Deal with cursor position
#if INTERNAL
		if(!input_playbacker.is_playback_input)
#endif
		{
			POINT cursor_pos = {};
			GetCursorPos(&cursor_pos);
			ScreenToClient(window, &cursor_pos);
			game_input.screen_mouse_pos.x = (u32)cursor_pos.x;
			game_input.screen_mouse_pos.y = (u32)cursor_pos.y;
			
			game_input.render_mouse_pos = game_input.screen_mouse_pos - render_region.min;
			Vec2f render_dims = { 
				(f32)Rect_Width(render_region), 
				(f32)Rect_Height(render_region)
			};
			
			Vec2f design_dims_f = Vec_Cast<f32>(design_dims);
			Vec2f design_to_render_ratio = Vec_Ratio(design_dims_f, render_dims);
			
			game_input.design_mouse_pos = 
        Vec_Cast<f32>(game_input.render_mouse_pos) * design_to_render_ratio;
			
			// NOTE(Momo): Flip y
			// TODO(Momo): should this really be here?
			game_input.design_mouse_pos.y = Lerp(design_dims.y, 
                                           0.f, 
                                           game_input.design_mouse_pos.y/design_dims.y);	
		}
		
		
		
		//- NOTE(Momo): Audio starts here
		Platform_Audio platform_audio_output = Win32_Audio_Begin(&audio);
		if (!platform_audio_output) {
			Win32_Log("[Win32] Problem beginning audio frame!\n");
			return 1;
		}
		
		//- NOTE(Momo): Rendering starts here
		Mailbox* render_commands = renderer_code.begin_frame(renderer, client_dims, render_region);
		if(!render_commands) {
			Win32_Log("[Win32] Problem beginning renderer frame!\n");
			return 1;
		}
		
		f32 game_dt = target_secs_per_frame;
		b8 is_game_running = true;
		if (game_code.update) {
			is_game_running = game_code.update(&game_memory.head,
                                         &platform_api,
                                         render_commands,
                                         &game_input,
                                         &platform_audio_output,
                                         game_dt);
			
		}
		state.is_running = is_game_running && state.is_running;
		
		//- NOTE(Momo): Rendering ends here
		renderer_code.end_frame(renderer, render_commands);
		
		//- NOTE(Momo): Audio ends here 
		Win32_Audio_End(&audio, platform_audio_output);
		
		f32 secs_elapsed = 
      Win32_GetSecondsElapsed(&state, last_count, Win32_GetPerformanceCounter());
		
		// NOTE(Momo): Sleep time
		if (target_secs_per_frame > secs_elapsed) {
			if (is_sleep_granular) {
				DWORD ms_to_sleep = 
				(DWORD)(1000.f * (target_secs_per_frame - secs_elapsed));
				
				// We cut the sleep some slack, so we sleep 1 ms less.
				if (ms_to_sleep > 1) {
					Sleep(ms_to_sleep - 1);
				}
			}
			while(target_secs_per_frame > secs_elapsed) {
				secs_elapsed = Win32_GetSecondsElapsed(&state, last_count, Win32_GetPerformanceCounter());
			}
		}
		
		last_count = Win32_GetPerformanceCounter();
		
		Win32_SwapBuffers(window);
		
	}
	
	return 0;
}



