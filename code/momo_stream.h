#ifndef __MM_STREAM__
#define __MM_STREAM__

struct Stream {
    u8* data;
    u32 data_size;
    u32 current;
    
	
    // For bit reading
    u32 bit_buffer;
    u32 bit_count;
    
    inline u8& operator[](u32 index) {
        Assert(index < data_size);
        return data[index];
    }
};


static inline b8
Stream_Init(Stream* s, void* memory, u32 memory_size) {
    if ( memory == nullptr || memory_size == 0) {
        return false;
    }
    s->data = (u8*)memory;
    s->data_size = memory_size;
    return true;
}



static inline b8
Stream_Alloc(Stream* s, Arena* arena, u32 capacity) {
    void* memory = Arena_PushBlock(arena, capacity);
    return Stream_Init(s, memory, capacity); 
} 

static inline void
Stream_Reset(Stream* s) {
    s->current = 0;
}

static inline b8
Stream_IsEos(Stream* s) {
    return s->current >= s->data_size;
}

static inline void*
Stream_ConsumeBlock(Stream* s, u32 amount) {
    void* ret = nullptr;
    if (s->current + amount <= s->data_size) {
        ret = s->data + s->current;
    }
    s->current += amount;
    return ret;
}

template<typename T>
static inline T*
Stream_Consume(Stream* s) {
    return (T*)Stream_ConsumeBlock(s, sizeof(T));
}

static inline b8
Stream_WriteBlock(Stream* s, void* src, u32 src_size) {
    if (s->current + src_size > s->data_size) {
        return false;
    }
    CopyBlock(s->data + s->current, src, src_size);
    s->current += src_size; 
    return true;
}

template<typename T>
static inline b8
Stream_Write(Stream* s, T item) {
    return Stream_WriteBlock(s, &item, sizeof(T));
}

static inline void
Stream_FlushBits(Stream* s){
	s->bit_buffer = 0;
	s->bit_count = 0;
}

// Bits are consumed from LSB to MSB
static inline u32
Stream_ConsumeBits(Stream* s, u32 amount){
    Assert(amount <= 32);
    
    while(s->bit_count < amount) {
        u32 byte = *Stream_Consume<u8>(s);
        s->bit_buffer |= (byte << s->bit_count);
        s->bit_count += 8;
    }
    
    u32 result = s->bit_buffer & ((1 << amount) - 1); 
    
    s->bit_count -= amount;
    s->bit_buffer >>= amount;
    
    return result;
}


#endif
