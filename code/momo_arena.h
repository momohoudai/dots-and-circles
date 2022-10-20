#ifndef __MOMO_ARENA__
#define __MOMO_ARENA__

struct Arena;
struct Arena_Marker {
    // NOTE(Momo): Regarding reverting, an arena status back to an Arena_Mark,
    // I considered using destructors, to return the arena but...
    // there are still cases where I feel that you do *not* want to destruct
    Arena* arena;
    u32 old_used;
    
    // TODO: Can we remove this?
    // Can't we just create more free functions specific to Arena_Mark itself? 
    // There are some functions from an API pov where
    // Arena_Mark shouldn't act like Areaa, for example: clear()
    inline operator Arena*() {
        return this->arena;
    }
    
};

struct Arena {
    u8* memory;
    u32 used;
    u32 capacity;
    
};

static b8 
Arena_Init(Arena* arena, void* mem, u32 cap);

static void 
Arena_Clear(Arena* arena);

static u32 
Arena_Remaining(Arena* arena);

static void* 
Arena_PushBlock(Arena* arena, 
                u32 size, 
                u8 alignment = alignof(void*));

template<typename T>
static T* 
Arena_Push(Arena* arena);

template<typename T>
static T*
Arena_PushArray(Arena* arena, 
                u32 count);

static b8
Arena_Partition(Arena* dest_arena, 
                Arena* src_arena, 
                u32 size);

static void* 
Arena_BootBlock(u32 struct_size,
                u32 offset_to_arena,
                void* memory,
                u32 memory_size);

static Arena_Marker
Arena_Mark(Arena* arena);

static void
Arena_Revert(Arena_Marker* mark);

#include "momo_arena.cpp"

#endif
