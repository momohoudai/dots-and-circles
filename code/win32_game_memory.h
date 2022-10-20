/* date = July 10th 2021 0:23 am */

#ifndef TARGET_WIN32_GAME_MEMORY_H
#define TARGET_WIN32_GAME_MEMORY_H

struct Win32_GameMemory {
    Game_Memory head;
    
    void* data;
    u32 data_size;
    
};

static inline void
Win32_GameMemory_Free(Win32_GameMemory* gm) {
    Win32_FreeMemory(gm->data);
}

static inline b8
Win32_GameMemory_Init(Win32_GameMemory* gm,
                      u32 permanent_memory_size) 
{
    gm->data_size = permanent_memory_size;
    gm->data = Win32_AllocateMemory(gm->data_size);

    if (!gm->data) {
		Win32_Log("[Win32::GameMemory] Failed to allocate memory\n");
        return false;
    }
    
    u8* memory_ptr = (u8*)gm->data;
    
    gm->head.permanent_memory_size = permanent_memory_size;
    gm->head.permanent_memory = gm->data;
    
    
    return true;
}

static inline b8
Win32_GameMemory_Save(Win32_GameMemory* gm, const char* path) {
    // We just dump the whole game memory into a file
    HANDLE win32_handle = CreateFileA(path,
                                      GENERIC_WRITE,
                                      FILE_SHARE_WRITE,
                                      0,
                                      CREATE_ALWAYS,
                                      0,
                                      0);
    if (win32_handle == INVALID_HANDLE_VALUE) {
		Win32_Log("[Win32::GameMemory] Invalid save path: %s\n", path);
        return false;
    }
    defer { CloseHandle(win32_handle); }; 
    
    DWORD bytes_written;
    if(!WriteFile(win32_handle, 
                  gm->data,
                  (DWORD)gm->data_size,
                  &bytes_written,
                  0)) 
    {
		Win32_Log("[Win32::GameMemory] Cannot write to file %s\n", path);
        return false;
    }
    
    if (bytes_written != gm->data_size) {
		Win32_Log("[Win32::GameMemory] Something went wrong writing to file: %s %d %d\n", 
				  path, gm->data_size, bytes_written);
        return false;
    }
    return true; 
    
}

static inline b8
Win32_GameMemory_Load(Win32_GameMemory* gm, const char* path) {
    HANDLE win32_handle = CreateFileA(path,
                                      GENERIC_READ,
                                      FILE_SHARE_READ,
                                      0,
                                      OPEN_EXISTING,
                                      0,
                                      0);
    if (win32_handle == INVALID_HANDLE_VALUE) {
		Win32_Log("[Win32::GameMemory] Invalid load path: %s\n", path);
        return false;
    }
    defer { CloseHandle(win32_handle); }; 
    DWORD bytes_read;
    
    BOOL success = ReadFile(win32_handle, 
                            gm->data,
                            (DWORD)gm->data_size,
                            &bytes_read,
                            0);
    
    if (success && gm->data_size == bytes_read) {
        return true;
    }
    
	Win32_Log("[Win32::GameMemory] Something went wrong loading file: %s %d %d\n", 
			  path, gm->data_size, bytes_read);
    return false;
}



#endif //TARGET_WIN32_GAME_MEMORY_H
