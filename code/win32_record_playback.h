/* date = July 10th 2021 0:49 pm */

#ifndef TARGET_WIN32_RECORD_PLAYBACK_H
#define TARGET_WIN32_RECORD_PLAYBACK_H


//~ NOTE(Momo): Input recorder
struct Win32_InputRecorder {
    b8 is_recording_input;
    HANDLE recording_input_handle;
};


static inline b8
Win32_InputRecorder_Begin(Win32_InputRecorder* r, const char* path) {
    if(r->is_recording_input){
		Win32_Log("[Win32::InputRecorder] Cannot begin. Is already recording!\n");
        return false;
    }
    HANDLE record_file_handle = CreateFileA(path,
                                            GENERIC_WRITE,
                                            FILE_SHARE_WRITE,
                                            0,
                                            CREATE_ALWAYS,
                                            0,
                                            0);
    
    if (record_file_handle == INVALID_HANDLE_VALUE) {
		Win32_Log("[Win32::InputRecorder] Path is invalid: %s\n", path);
        return false;
    }
    r->recording_input_handle = record_file_handle;
    r->is_recording_input = true;
	
    return true;
}

static inline b8
Win32_InputRecorder_End(Win32_InputRecorder* r) {
    if (!r->is_recording_input) {
		Win32_Log("[Win32::InputRecorder] Cannot end. Is not recording!\n");
        return false;
    }
    CloseHandle(r->recording_input_handle);
    r->is_recording_input = false;
    
    return true;
}

static inline b8
Win32_InputRecorder_Update(Win32_InputRecorder* r, Platform_Input* input) {
    if(!r->is_recording_input) {
        return false;
    }
    DWORD bytes_written;
    if(!WriteFile(r->recording_input_handle,
                  input,
                  sizeof(Platform_Input),
                  &bytes_written, 0)) 
    {
		Win32_Log("[Win32::InputRecorder] Cannot write to file\n");
        Win32_InputRecorder_End(r);
        return false;
    }
    
    if (bytes_written != sizeof(Platform_Input)) {
		Win32_Log("[Win32::InputRecorder] Something went wrong writing to file\n");
        Win32_InputRecorder_End(r);
        return false;
    }
    return true;
}

//~ NOTE(Momo): Playbacker
struct Win32_InputPlaybacker {
    b8 is_playback_input;
    HANDLE playback_input_handle;
    
};

static inline b8
Win32_InputPlaybacker_End(Win32_InputPlaybacker* p) {
    if(!p->is_playback_input) {
		Win32_Log("[Win32::Playbacker] Cannot end. Is not playing back!\n");
        return false;
    }
    CloseHandle(p->playback_input_handle);
    p->is_playback_input = false;
    return true;
}

static inline b8
Win32_InputPlaybacker_Begin(Win32_InputPlaybacker* p, const char* path) {
    if(p->is_playback_input){
		Win32_Log("[Win32::Playbacker] Cannot begin. Is already playing back!\n");
        return false;
    }
    HANDLE record_file_handle = CreateFileA(path,
                                            GENERIC_READ,
                                            FILE_SHARE_READ,
                                            0,
                                            OPEN_EXISTING,
                                            0,
                                            0);
    
    if (record_file_handle == INVALID_HANDLE_VALUE) {
		Win32_Log("[Win32::InputPlaybacker] Path is invalid: %s\n", path);
        return false;
    }
    p->playback_input_handle = record_file_handle;
    p->is_playback_input = true;
    return true;
}

// NOTE(Momo): returns true if 'done' reading all input, false otherwise
static inline b8 
Win32_InputPlaybacker_Update(Win32_InputPlaybacker* p, Platform_Input* input) {
    DWORD bytes_read;
    BOOL success = ReadFile(p->playback_input_handle, 
                            input,
                            sizeof(Platform_Input),
                            &bytes_read,
                            0);
    if(!success || bytes_read != sizeof(Platform_Input)) {
        return true;
    }
	Win32_Log("[Win32::InputPlaybacker] Problem reading file\n");
	Win32_InputPlaybacker_End(p);
    return false;
}

#endif //TARGET_WIN32_RECORD_PLAYBACK_H
