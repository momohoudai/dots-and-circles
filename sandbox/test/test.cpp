// NOTE(Momo): Yet another attempt to write a bmp loader
// for formats I am interested in. 
// 


#include <stdio.h>
#include <stdlib.h>
#include "../../code/momo.h"
#include "../../code/momo_png.h"

static inline Memory_Block
ReadFileIntoMemory(const char* filename) {
    FILE* file = nullptr;
    fopen_s(&file, filename, "rb");
    
    if (file == nullptr) return {};
    defer { fclose(file); };
    
    fseek(file, 0, SEEK_END);
    u32 size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    void* buffer = malloc(size);
    fread(buffer, size, 1, file);
    
    return { buffer, size };
}


static inline b8
WriteMemoryIntoFile(const char* filename, Memory_Block memory) {
    if (!memory) return false;
    
    FILE * file = nullptr;
    fopen_s(&file, filename, "wb");
    if (file == nullptr) return false;
    defer { fclose(file); };
    fwrite(memory.data, memory.size, 1, file);
    
    return true;
}


int main() {
    auto test = Vec_Create(1,2,3);
    printf("%d%d%d\n", test.x, test.y, test.z);
    Arena arena = {};
    
    auto png_file = ReadFileIntoMemory("test.png");
    if (!png_file) {
        printf("Cannot find test.png\n");
        return 1;
    }
    defer { free(png_file.data); };
    
    const u32 arena_memory_size = Mebibytes(1);
    void* arena_memory = malloc(arena_memory_size);
    if (!arena_memory) {
        printf("Cannot allocate memory\n");
        return 1;
    }
    
    defer { free(arena_memory); };
    
    if(!Arena_Init(&arena, arena_memory, arena_memory_size)) {
        printf("Cannot init arena\n");
        return 1;
    }
    
    auto png = PNG_LoadFromMemory(png_file.data, png_file.size, &arena);
    printf("Error Code: %d\n", png.err);
    
    
#if 0
    auto png_output = PNG_WriteToMemory(png.width, png.height, png.channels, png.data, &arena);
    if (!png_output) {
        printf("cannot output png\n");
    }
    WriteMemoryIntoFile("output.png", png_output);
#endif
    
    
    
    
    system("pause");
}