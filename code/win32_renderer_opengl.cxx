// NOTE(Momo): Renderer code for Win32 w/ Opengl


#include "win32_renderer.h"
#include "game_renderer_opengl.h"


//~ NOTE(Momo): WGL stuff
#define WGL_CONTEXT_MAJOR_VERSION_ARB           0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB           0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB             0x2093
#define WGL_CONTEXT_FLAGS_ARB                   0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB            0x9126

#define WGL_DRAW_TO_WINDOW_ARB                  0x2001
#define WGL_ACCELERATION_ARB                    0x2003
#define WGL_SUPPORT_OPENGL_ARB                  0x2010
#define WGL_DOUBLE_BUFFER_ARB                   0x2011
#define WGL_PIXEL_TYPE_ARB                      0x2013
#define WGL_TYPE_RGBA_ARB                       0x202B
#define WGL_FULL_ACCELERATION_ARB               0x2027
#define WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB        0x20A9
#define WGL_CONTEXT_FLAG_ARB                    0x2094
#define WGL_CONTEXT_DEBUG_BIT_ARB               0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB  0x0002
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB        0x00000001

#define WGL_FUNCTION_DECL(Name) wgl_func_##Name
#define WGL_FUNCTION_DECL_PTR(Name) WGL_FUNCTION_DECL(Name)* Name
typedef BOOL WINAPI 
WGL_FUNCTION_DECL(wglChoosePixelFormatARB)(HDC hdc,
                                           const int* piAttribIList,
                                           const FLOAT* pfAttribFList,
                                           UINT nMaxFormats,
                                           int* piFormats,
                                           UINT* nNumFormats);

typedef BOOL WINAPI 
WGL_FUNCTION_DECL(wglSwapIntervalEXT)(int interval);

typedef HGLRC WINAPI 
WGL_FUNCTION_DECL(wglCreateContextAttribsARB)(HDC hdc, 
                                              HGLRC hShareContext,
                                              const int* attribList);

typedef const char* WINAPI 
WGL_FUNCTION_DECL(wglGetExtensionsStringEXT)();

static WGL_FUNCTION_DECL_PTR(wglCreateContextAttribsARB);
static WGL_FUNCTION_DECL_PTR(wglChoosePixelFormatARB);
static WGL_FUNCTION_DECL_PTR(wglSwapIntervalEXT);
//static WGL_FUNCTION_DECL_PTR(wglGetExtensionsStringEXT);


static inline void* 
WGL_TryGetFunction(const char* name, HMODULE fallback_module)
{
    void* p = (void*)wglGetProcAddress(name);
    if ((p == 0) || 
        (p == (void*)0x1) || 
        (p == (void*)0x2) || 
        (p == (void*)0x3) || 
        (p == (void*)-1))
    {
        p = (void*)GetProcAddress(fallback_module, name);
    }
    return p;
    
}

static inline void
WGL_SetPixelFormat(HDC dc) {
    s32 suggested_pixel_format_index = 0;
    u32 extended_pick = 0;
    
    if (wglChoosePixelFormatARB) {
        s32 attrib_list[] = {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
            WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
            WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE,
            0,
        };
        
        wglChoosePixelFormatARB(dc, attrib_list, 0, 1,
                                &suggested_pixel_format_index, &extended_pick);
        
    }
    
    if (!extended_pick) {
        PIXELFORMATDESCRIPTOR desired_pixel_format = {};
        desired_pixel_format.nSize = sizeof(desired_pixel_format);
        desired_pixel_format.nVersion = 1;
        desired_pixel_format.iPixelType = PFD_TYPE_RGBA;
        desired_pixel_format.dwFlags = 
            PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER; 
        desired_pixel_format.cColorBits = 32;
        desired_pixel_format.cAlphaBits = 8;
        desired_pixel_format.iLayerType = PFD_MAIN_PLANE;
        
        // Here, we ask windows to find the best supported pixel 
        // format based on our desired format.
        suggested_pixel_format_index = 
            ChoosePixelFormat(dc, &desired_pixel_format);
    }
    PIXELFORMATDESCRIPTOR suggested_pixel_format = {};
    
    DescribePixelFormat(dc, suggested_pixel_format_index, 
                        sizeof(suggested_pixel_format), 
                        &suggested_pixel_format);
    SetPixelFormat(dc, 
                   suggested_pixel_format_index, 
                   &suggested_pixel_format);
}
static inline b8
WGL_LoadExtensions() {
    WNDCLASSA window_class = {};
    // Er yeah...we have to create a 'fake' Opengl context 
    // to load the extensions lol.
    window_class.lpfnWndProc = DefWindowProcA;
    window_class.hInstance = GetModuleHandle(0);
    window_class.lpszClassName = "WGLLoader2";
    
    if (RegisterClassA(&window_class)) {
        HWND window = CreateWindowExA( 
                                      0,
                                      window_class.lpszClassName,
                                      "WGL Loader2",
                                      0,
                                      CW_USEDEFAULT,
                                      CW_USEDEFAULT,
                                      CW_USEDEFAULT,
                                      CW_USEDEFAULT,
                                      0,
                                      0,
                                      window_class.hInstance,
                                      0);
        defer { DestroyWindow(window); };
        
        HDC dc = GetDC(window);
        defer { ReleaseDC(window, dc); };
        
        WGL_SetPixelFormat(dc);
        
        HGLRC opengl_context = wglCreateContext(dc);
        defer { wglDeleteContext(opengl_context); };
        
        
        if (wglMakeCurrent(dc, opengl_context)) {
            
#define WIN32_SET_WGL_FUNCTION_DECL(Name) \
Name = (WGL_FUNCTION_DECL(Name)*)wglGetProcAddress(#Name); \
if (!Name) { \
return false; \
}
            
            WIN32_SET_WGL_FUNCTION_DECL(wglChoosePixelFormatARB);
            WIN32_SET_WGL_FUNCTION_DECL(wglCreateContextAttribsARB);
            WIN32_SET_WGL_FUNCTION_DECL(wglSwapIntervalEXT);
#undef WIN32_SET_WGL_FUNCTION_DECL            
            
			wglMakeCurrent(0, 0);
            return true;
        }
        else {
            return false;
        }
        
    }
    else {
		DWORD test = GetLastError();
        return false;
    }
}
static inline void*
Win32_Renderer_AllocateMemory(umi memory_size) {
    return VirtualAllocEx(GetCurrentProcess(),
                          0, 
                          memory_size,
                          MEM_RESERVE | MEM_COMMIT, 
                          PAGE_READWRITE);
    
}

static inline void
Win32_Renderer_FreeMemory(void* memory) {
	VirtualFreeEx(GetCurrentProcess(), 
				  memory,    
				  0, 
				  MEM_RELEASE); 
}

						
//~ NOTE(Momo): Exported functions shared across all win32 renderers
extern "C" 
Win32_Renderer_DeclareInitFunction(Renderer_Init) {
	// NOTE(Momo): Calcluate the EXACT amount of memory needed
	u32 max_textures = 8;
	u32 memory_required = sizeof(Opengl) + sizeof(GLuint)*max_textures + render_commands_size;
	
	void* memory = Win32_Renderer_AllocateMemory(memory_required);
	if (!memory) {
		return nullptr;
	}
	Opengl* opengl = Arena_Boot(Opengl, arena, memory, memory_required);
	
	HDC device_ctx = GetDC(window); 
    defer { ReleaseDC(window, device_ctx); };
    
    if (!WGL_LoadExtensions()) {
        return nullptr;
    }
    
	WGL_SetPixelFormat(device_ctx);
    
    
    s32 opengl_attribs[] {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 5,
        WGL_CONTEXT_FLAG_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB
#if INTERNAL
        | WGL_CONTEXT_DEBUG_BIT_ARB
#endif
        ,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0,
    };
    HGLRC opengl_ctx = wglCreateContextAttribsARB(device_ctx, 0, 
                                                     opengl_attribs); 
    
    if (!opengl_ctx) {
        return nullptr;
    }
	
	
	
	
    
    if(wglMakeCurrent(device_ctx, opengl_ctx)) {
        HMODULE Module = LoadLibraryA("opengl32.dll");
        // TODO: Log functions that are not loaded
#define WGL__SetOpenglFunction(Name) \
opengl->Name = (Opengl_DeclareOpenglFunction(Name)*)WGL_TryGetFunction(#Name, Module); \
if (!opengl->Name) { \
return nullptr; \
}
        
        WGL__SetOpenglFunction(glEnable);
        WGL__SetOpenglFunction(glDisable); 
        WGL__SetOpenglFunction(glViewport);
        WGL__SetOpenglFunction(glScissor);
        WGL__SetOpenglFunction(glCreateShader);
        WGL__SetOpenglFunction(glCompileShader);
        WGL__SetOpenglFunction(glShaderSource);
        WGL__SetOpenglFunction(glAttachShader);
        WGL__SetOpenglFunction(glDeleteShader);
        WGL__SetOpenglFunction(glClear);
        WGL__SetOpenglFunction(glClearColor);
        WGL__SetOpenglFunction(glCreateBuffers);
        WGL__SetOpenglFunction(glNamedBufferStorage);
        WGL__SetOpenglFunction(glCreateVertexArrays);
        WGL__SetOpenglFunction(glVertexArrayVertexBuffer);
        WGL__SetOpenglFunction(glEnableVertexArrayAttrib);
        WGL__SetOpenglFunction(glVertexArrayAttribFormat);
        WGL__SetOpenglFunction(glVertexArrayAttribBinding);
        WGL__SetOpenglFunction(glVertexArrayBindingDivisor);
        WGL__SetOpenglFunction(glBlendFunc);
        WGL__SetOpenglFunction(glCreateProgram);
        WGL__SetOpenglFunction(glLinkProgram);
        WGL__SetOpenglFunction(glGetProgramiv);
        WGL__SetOpenglFunction(glGetProgramInfoLog);
        WGL__SetOpenglFunction(glVertexArrayElementBuffer);
        WGL__SetOpenglFunction(glCreateTextures);
        WGL__SetOpenglFunction(glTextureStorage2D);
        WGL__SetOpenglFunction(glTextureSubImage2D);
        WGL__SetOpenglFunction(glBindTexture);
        WGL__SetOpenglFunction(glTexParameteri);
        WGL__SetOpenglFunction(glBindVertexArray);
        WGL__SetOpenglFunction(glDrawElementsInstancedBaseInstance);
        WGL__SetOpenglFunction(glGetUniformLocation);
        WGL__SetOpenglFunction(glProgramUniformMatrix4fv);
        WGL__SetOpenglFunction(glNamedBufferSubData);
        WGL__SetOpenglFunction(glUseProgram);
        WGL__SetOpenglFunction(glDeleteTextures);
        WGL__SetOpenglFunction(glDebugMessageCallbackARB);
    }
#undef WGL__SetOpenglFunction
	
	
	
    if (!Opengl_Init(opengl, 8,render_commands_size))
	{
		return nullptr;
	}
    
#if INTERNAL
    opengl->glEnable(GL_DEBUG_OUTPUT);
    opengl->glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	// TODO(Momo): Figure out how to get callback?
	//    g_opengl.glDebugMessageCallbackARB(Win32_Opengl_DebugCallback, nullptr);
#endif
    
	
    return opengl;
}


extern "C" 
Win32_Renderer_DeclareBeginFrameFunction(Renderer_BeginFrame) {
	return Opengl_BeginFrame((Opengl*)renderer, render_wh, render_region);
}


extern "C" 
Win32_Renderer_DeclareEndFrameFunction(Renderer_EndFrame) {
	Opengl_EndFrame((Opengl*)renderer, render_commands);
}


extern "C"
Win32_Renderer_DeclareFreeFunction(Renderer_Free) {
	Opengl* opengl = (Opengl*)renderer;
	Win32_Renderer_FreeMemory(opengl->arena.memory);
}

extern "C"
Win32_Renderer_DeclareClearTexturesFunction(Renderer_ClearTextures) {
	Opengl_ClearTextures((Opengl*)renderer);
}

extern "C"
Win32_Renderer_DeclareAddTextureFunction(Renderer_AddTexture) {
	return Opengl_AddTexture((Opengl*)renderer, width, height, pixels);
}
