#ifndef __MOMO_PNG__
#define __MOMO_PNG__

struct PNG {
    u32 width;
    u32 height; 
    u32 channels;
    void* data;
    
    inline operator bool() {
        return data != nullptr;
    }
};


static PNG PNG_Read(Memory_Block png_memory, Arena* arena);
static Memory_Block PNG_Write(PNG image, Arena* arena);


#endif //MOMO_PNG_H
