/* date = March 14th 2021 2:39 pm */

#ifndef TOOL_BUILD_ASSETS_UTIL_H
#define TOOL_BUILD_ASSETS_UTIL_H

struct Read_Font_Result {
	stbtt_fontinfo info;
    void* info_data;
    
    // NOTE(Momo): Precomputed for ease of use 
    f32 pixel_scale; // scale for 1 pixel worth of height
    f32 raster_scale;
    Font_ID font_id;
    Texture_ID texture_id;
};

static inline Memory_Block
ReadFileIntoMemory(const char* filename, Arena *arena) {
    Memory_Block ret = {};
    FILE* File = nullptr;
    fopen_s(&File, filename, "rb");
    
    if (File == nullptr) {
        return {};
    }
    defer { fclose(File); };
    
    
    fseek(File, 0, SEEK_END);
    u32 size = ftell(File);
    fseek(File, 0, SEEK_SET);
    
    void* buffer = Arena_PushBlock(arena, size);
    fread(buffer, size, 1, File);
    
    ret.data = buffer;
    ret.size = size;
    
    return ret;
}

#endif //TOOL_BUILD_ASSETS_UTIL_H
