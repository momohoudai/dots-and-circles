// This layer is basically a bunch of settings required by both 
// the game and platform.
#ifndef __PLATFORM__
#define __PLATFORM__



// Input API /////////////////////////////////////////
struct Platform {};

struct Platform_Input_Button {
    b8 before : 1;
    b8 now : 1;
    
};

struct Platform_Input {
#if INTERNAL
	u8 characters_buffer[10];
    StringBuffer characters;
#endif // INTERNAL
	
    union {
#if INTERNAL
        Platform_Input_Button buttons[8];
#else
		Platform_Input_Button buttons[2];
#endif // INTERNAL
        struct {
            Platform_Input_Button button_switch;
			Platform_Input_Button button_pause;
            
#if INTERNAL
            // NOTE(Momo): Kinda for in-game debugging
            Platform_Input_Button button_confirm;
            Platform_Input_Button button_back;
            Platform_Input_Button button_console;
            Platform_Input_Button button_inspector;
            Platform_Input_Button button_speed_up;
            Platform_Input_Button button_speed_down;
#endif // INTERNAL
        };
    };
    
    
    Vec2f design_mouse_pos; // Position of mouse in game
    Vec2u screen_mouse_pos; // Position of mouse to window region
    Vec2u render_mouse_pos; // Position of mouse to render region
    
};


#if INTERNAL
static inline b8
Platform_Input_Init(Platform_Input* p) {
    return StringBuffer_Init(&p->characters, 
							 p->characters_buffer, 
							 ArrayCount(p->characters_buffer));
}


static inline b8
Platform_Input_PushChar(Platform_Input* p ,u8 c) {
    if (c >= 32 && c <= 126) {
        StringBuffer_PushU8(&p->characters, c);
        return true;
    }
    return false;
}
#endif // INTERNAL

static inline void
Platform_Input_Update(Platform_Input* p) {	
#if INTERNAL
    StringBuffer_Clear(&p->characters);
#endif // INTERNAL
	
    for (auto&& itr : p->buttons) {
        itr.before = itr.now;
    }
}

// before: 0, now: 1
static inline b8 
Platform_Input_IsButtonPoked(Platform_Input_Button b) {
    return !b.before && b.now;
}

// before: 1, now: 0
static inline b8
Platform_Input_IsButtonReleased(Platform_Input_Button b) {
    return b.before && !b.now;
}


// before: X, now: 1
static inline b8
Platform_Input_IsButtonDown(Platform_Input_Button b) {
    return b.now;
}

// before: 1, now: 1
static inline b8
Platform_Input_IsButtonHeld(Platform_Input_Button b) {
    return b.before && b.now;
}

// Platform Api ////////////////////////////////////////////////////
struct Platform_File_Handle {
    u32 id;
    u32 error; // 0 is always no error!
};

// NOTE(Momo): The 'ways' of opening files that we are interested in:
// - Reading. Opens an existing file for reading.
// - Overwrite. Opens an existing 
enum Platform_File_Open_Mode {
    Platform_File_Open_Mode_Read, 
    Platform_File_Open_Mode_Overwrite,
};

enum Platform_File_Location {
	Platform_File_Location_Game,
	Platform_File_Location_User,
};

#define Platform_DeclareLogFunction(Name) void Name(const char* format, ...)
typedef Platform_DeclareLogFunction(Platform_Log);

#define Platform_DeclareCloseFileFunction(Name) void Name(Platform_File_Handle* handle)
typedef Platform_DeclareCloseFileFunction(Platform_Close_File);

#define Platform_DeclareReadFileFunction(Name) void Name(Platform_File_Handle* handle, u64 offset, u64 size, void* dest)
typedef Platform_DeclareReadFileFunction(Platform_Read_File);

#define Platform_DeclareWriteFileFunction(Name) void Name(Platform_File_Handle* handle, u64 offset, u64 size, void* src)
typedef Platform_DeclareWriteFileFunction(Platform_Write_File);

#define Platform_DeclareGetFileSizeFunction(Name) u32 Name(const char* path)
typedef Platform_DeclareGetFileSizeFunction(Platform_Get_File_Size);

#define Platform_DeclareLogFileErrorFunction(Name) void Name(Platform_File_Handle* handle)
typedef Platform_DeclareLogFileErrorFunction(Platform_Log_File_Error);

#define Platform_DeclareAddTextureFunction(Name) Renderer_Texture_Handle Name(u32 width, u32 height, void* pixels)
typedef Platform_DeclareAddTextureFunction(Platform_Add_Texture);

#define Platform_DeclareClearTexturesFunction(Name) void Name()
typedef Platform_DeclareClearTexturesFunction(Platform_Clear_Textures);

#define Platform_DeclareHideCursorFunction(Name) void Name()
typedef Platform_DeclareHideCursorFunction(Platform_Hide_Cursor);

#define Platform_DeclareShowCursorFunction(Name) void Name()
typedef Platform_DeclareShowCursorFunction(Platform_Show_Cursor);

#define Platform_DeclareGetPerformanceCounterFunction(Name) u64 Name()
typedef Platform_DeclareGetPerformanceCounterFunction(Platform_Get_Performance_Counter);

// TODO: Should use String
#define Platform_DeclareOpenFileFunction(Name) Platform_File_Handle Name(const char* file, Platform_File_Open_Mode  open_mode, Platform_File_Location location)
typedef Platform_DeclareOpenFileFunction(Platform_Open_File);


#if INTERNAL
#define Platform_DeclareGetFileModifiedTimeFunction(Name) u64 Name(const char* filename)
typedef Platform_DeclareGetFileModifiedTimeFunction(Platform_GetFileModifiedTime);
#endif // INTERNAL

struct Platform_API {    
    // NOTE(Momo): File API
    Platform_Open_File* open_file;
    Platform_Close_File* close_file;
    Platform_Log_File_Error* log_file_error;
    Platform_Read_File* read_file;
    Platform_Write_File* write_file;
	
#if INTERNAL	
	Platform_GetFileModifiedTime *get_file_modified_time;
#endif // INTERNAL
	
    // NOTE(Momo): Cursor API
    Platform_Show_Cursor* show_cursor;
    Platform_Hide_Cursor* hide_cursor;
    
    // NOTE(Momo): Misc
    Platform_Log* log;
    Platform_Get_File_Size* get_file_size;
    Platform_Add_Texture* add_texture;
    Platform_Clear_Textures* clear_textures;
    Platform_Get_Performance_Counter* get_performance_counter;
};

// Memory required by the game to get it running
// Note that Transient Memory, once set, should not be moved!
// TODO(Momo): should change name to "platform_memory" because it is
// memory FROM the platform
struct Game_Memory {
    void* permanent_memory;
    u32 permanent_memory_size;
    
    void* scratch_memory;
    u32 scratch_memory_size;
    
    
};

struct Platform_Audio {
    s16* sample_buffer;
    u32 sample_count;
    u32 channels;
	inline operator b8() {
		return sample_buffer != nullptr && sample_count;
	}
};



//~ NOTE(Momo): game.dll functions

#define Game_DeclareGameUpdateFunction(Name) b8 Name(Game_Memory* game_memory, \
Platform_API* platform_api, \
Mailbox* render_commands, \
Platform_Input* platform_input, \
Platform_Audio* platform_audio,\
f32 dt)
typedef Game_DeclareGameUpdateFunction(Game_Update_Func);

struct Game_Functions {
	Game_Update_Func* update;
	
	
	static inline const char* names[] ={
		"Game_Update"
	};
};


//~ NOTE(Momo): Platform helper functions
static inline void*
Platform_ReadBlock(Platform_API* platform,
                   Platform_File_Handle* file,
                   Arena* arena,
                   u32* file_offset,
                   u32 block_size,
                   u8 block_alignment)
{
    void* ret = Arena_PushBlock(arena, block_size, block_alignment);
    if(!ret) {
        return nullptr; 
    }
    platform->read_file(file,
                        (*file_offset),
                        block_size,
                        ret);
    (*file_offset) += block_size;
    if (file->error) {
        platform->log_file_error(file);
        return nullptr;
    }
    return ret;
}

template<typename T>
static inline T*
Platform_Read(Platform_API* platform, Platform_File_Handle* file, Arena* arena, u32* file_offset) {
    return (T*)Platform_ReadBlock(platform, file, arena, file_offset, sizeof(T), alignof(T));
}


#endif //PLATFORM_H
