/* date = July 10th 2021 3:23 pm */

#ifndef TARGET_WIN32_GAME_CODE_H
#define TARGET_WIN32_GAME_CODE_H

// NOTE(Momo): Using templates might be overkill
// but without using it, the way to use this code
// is a bit unwieldy and unexpressive.
// You would have to create a seperate struct
// containing the functions pointers and function names 
// and use BOTH of them in tedium. For example:
// 
// Game_FunctionTable function_table = {}
// Loaded_Code loaded_code = {};
// loaded_code.function_table = function_table;
// loaded_code.function_names = function_names;
// Load(&loaded_code); 
// 
// if (loaded_code.is_valid) {
// 	// this part is the strange part.
// 	// we check loaded_code to use function_table...?
// 	function_table.some_exported_func();
// }
// 
// Our T needs to have an array of static const char* that 
// contains the names of the exported functions (i.e. code
// to import). The functions pointers must also appear first,
// and must be in the same order.
//

template<typename T>
struct Win32_ImportedCode : T {
	b8 is_valid;
	
  HMODULE dll;
  LARGE_INTEGER prev_write_time;
  
  const char *src_filename;
	
#if INTERNAL
  const char *temp_filename;
#endif // INTERNAL
	
  const char *lock_filename;
};

template<typename T>
static inline void
Win32_ImportedCode_Init(Win32_ImportedCode<T>* imported_code,
                        const char* src_filename,
#if INTERNAL
                        const char* temp_filename,
#endif // INTERNAL
                        const char* lock_filename)
{
	
#if INTERNAL
	imported_code->temp_filename = temp_filename;
#endif // INTERNAL
  imported_code->lock_filename = lock_filename;
  imported_code->src_filename = src_filename;
	
}


template<typename T>
static inline b8
Win32_ImportedCode_IsOutdated(Win32_ImportedCode<T>* c) {    
  // Check last modified date
  LARGE_INTEGER last_write_time = Win32_GetFileLastWriteTime(c->src_filename);
  return (last_write_time.QuadPart > c->prev_write_time.QuadPart); 
}

template<typename T>
static inline void 
Win32_ImportedCode_Unload(Win32_ImportedCode<T>* c) {
  if (c->dll) {
    FreeLibrary(c->dll);
    c->dll = 0;
  }
	
	void** functions = (void**)c;
	for (u32 i = 0; i < ArrayCount(c->names); ++i) {
		functions[i] = 0;
	}
	
  
}

template<typename T>
static inline b8
Win32_ImportedCode_Load(Win32_ImportedCode<T>* c) 
{
  WIN32_FILE_ATTRIBUTE_DATA ignored; 
  if(!GetFileAttributesEx(c->lock_filename, 
                          GetFileExInfoStandard, 
                          &ignored)) 
  {
    BOOL success = FALSE;
		
		
		
#if INTERNAL
		// TODO(Momo): I don't think this is right.
		// We should retry based on some timed interval?
    for(u32 i = 0; i < 256; ++i) {
			Win32_Log("[Win32::ImportedCode] Attempt #%d: Trying to copy %s\n", i, c->src_filename);
      success = CopyFile(c->src_filename, c->temp_filename, FALSE); 
			if(success) 
				break;
    }
		if (!success) { 
			Win32_Log("[Win32::ImportedCode] Fail to copy dll: %s", c->src_filename);
			return false;
		}
    c->dll = LoadLibraryA(c->temp_filename);
#else // !INTERNAL
		
		c->dll = LoadLibraryA(c->src_filename);
#endif // INTERNAL
		
    if(c->dll) {
			c->is_valid = true;
			void** functions = (void**)c;
			for(u32 i = 0; i < ArrayCount(c->names); ++i) {
				functions[i] = (void*)GetProcAddress(c->dll, c->names[i]);	
			}
      
			// NOTE(Momo): check if valid
			for(u32 i = 0; i < ArrayCount(c->names); ++i) {
				if (functions[i] == 0) {
					c->is_valid = false;
				}
			}
			Win32_Log("[Win32::ImportedCode] Loaded dll: %s\n", c->src_filename);
			c->prev_write_time = Win32_GetFileLastWriteTime(c->src_filename);
      return c->is_valid;
    }
		Win32_Log("[Win32::ImportedCode] Cannot load dll: %s\n", c->src_filename);
		return false;
		
  }
	Win32_Log("[Win32::ImportedCode] Failed to lock for: %s\n", c->src_filename);
	return false;
}

#endif //TARGET_WIN32_GAME_CODE_H
