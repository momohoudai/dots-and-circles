/* date = August 29th 2021 2:46 pm */

#ifndef TARGET_WIN32_LOGGER_H
#define TARGET_WIN32_LOGGER_H


struct Win32_Logger {
    HANDLE log_handle;
    
#if INTERNAL
    HANDLE std_out;
#endif
};

static inline b8
Win32_Logger_Init(Win32_Logger* l, const char* path) {
    
    HANDLE handle = CreateFileA(path,
                                GENERIC_WRITE,
                                FILE_SHARE_WRITE,
                                0,
                                CREATE_ALWAYS,
                                0,
                                0);
    
    if (handle == INVALID_HANDLE_VALUE) {
        return false;
    }
    
    
#if INTERNAL
    AllocConsole();    
    l->std_out = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
    l->log_handle = handle;
    return true;
}

static inline void
Win32_Logger_Free(Win32_Logger* l) {
#if INTERNAL
    FreeConsole();
#endif
    CloseHandle(l->log_handle);
}


#if INTERNAL
static inline void
Win32_Logger__WriteToConsole(Win32_Logger* l, String message) {
    
    WriteConsoleA(l->std_out,
                  message.data, 
                  message.count, 
                  0, 
                  NULL);
}
#endif

static inline b8
Win32_Logger__WriteToFile(Win32_Logger* l, String message) {
    DWORD bytes_written;
    if(!WriteFile(l->log_handle,
                  message.data,
                  message.count,
                  &bytes_written, 0)) 
    {
        return false;
    }
    
    if (bytes_written != message.count) {
        return false;
    }
    return true;
    
}

static inline void 
Win32_Logger_Write(Win32_Logger* l, String message) {
#if INTERNAL
    Win32_Logger__WriteToConsole(l, message);
#endif 
    Win32_Logger__WriteToFile(l, message);
}

#endif //TARGET_WIN32_LOGGER_H
