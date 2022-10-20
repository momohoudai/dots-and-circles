#ifndef __MOMO_BITWISE__
#define __MOMO_BITWISE__

// Note that all alignment operations is catered to only power of 2!
// Reference: https://stackoverflow.com/questions/227897/how-to-alloc-aligned-memory-only-using-the-standard-library
// 
// Let addressToAlign = A, byteToAlign = N
// To align A to N-byte is to align A to a byte that is 2^N.
// e.g. To align 0011 to 4-byte -> the result should be 0100 = 4
// e.g. To align 0110 to 4-byte -> the result should be 1000 = 8
//
// To align, we need to remove the log2(N) least significant bits from A.
// This means that for 2 byte alignment, we want to remove ONLY the last bits and retain the rest.
// 2 byte alignment -> remove least significant 1 bit.
// 4 byte alignment -> remove least significant 2 bits.
// 8 byte alignment -> remove least significant 3 bits.
// e.g. To align to 2 bytes (0010), we need to do A & (1110). 
// e.g. To align to 4 bytes (0100), we need to do A & (1100). 
// e.g. To align to 8 bytes (1000), we need to do A & (1000).
// and so on and so forth...
// Thus, the 'mask' can be defined as ~(N-1) 
// e.g.  2 bytes -> ~(0010 - 1) = ~0001 = 1110
// e.g.  4 bytes -> ~(0100 - 1) = ~0011 = 1100
// e.g.  8 bytes -> ~(1000 - 1) = ~0111 = 1000
// And thus, the forumla for backward alignment is: A & ~(N-1)
static inline void* 
AlignMemoryBackward(void* ptr, u8 align) {
    Assert(align > 0 && (align & (align - 1)) == 0); // power of 2 only
    return (void*)(umi(ptr) & ~(align - 1));
}

// The functions below return the difference in bytes between the given addresses and 
// the alignment they need to align to (backwards)
// If the forward alignment formula is: (A & ~(N-1))
// Then the formula for the difference is the the original address minus the result: 
// A - (A & ~(N-1))
static inline void* 
AlignMemoryForward(void* ptr, u8 align) {
    Assert(align > 0 && (align & (align - 1)) == 0); // power of 2 only
    return (void*)((umi(ptr) + (align - 1)) & ~(align - 1));
}

static inline u8 
AlignMemoryBackwardDiff(void* ptr, u8 align)  {
    return u8((umi)ptr - umi(AlignMemoryBackward(ptr, align)));
}

static inline u8 
AlignMemoryForwardDiff(void* ptr, u8 align)  {
    return u8(umi(AlignMemoryForward(ptr, align)) - umi(ptr));
}

static inline void 
CopyBlock(void* dest, void* src, umi size) {
    for (u8 *p = (u8*)dest, *q = (u8*)src, *e = p + size; 
         p < e; ++p, 
         ++q)
    {
        *p = *q;
    }
}

static inline void 
ZeroBlock(void *mem, u32 size) {
    for (u8 *p = (u8*)mem, *e = p + size; 
         p < e; 
         ++p)
    {
        *p = 0;
    }
}

#define ZeroStruct(p) ZeroBlock((p), sizeof(*(p)))
#define ZeroArray(a) ZeroBlock((a), sizeof((a)))
#define ZeroDynamicArray(a, c) ZeroBlock((a), sizeof(*(a)) * c)


static inline u32
constexpr FourCC(const char str[5]) {
    return 
    ((u32)(str[0]) << 0 ) |
    ((u32)(str[1]) << 8 ) |
    ((u32)(str[2]) << 16) |
    ((u32)(str[3]) << 24);
}

static inline u16
EndianSwapU16(u16 value) {
    return (value << 8) | (value >> 8);
}

static inline u32
EndianSwapU32(u32 value) {
    return  ((value << 24) |
			 ((value & 0xFF00) << 8) |
			 ((value >> 8) & 0xFF00) |
			 (value >> 24));
	
}

static inline void*
ConsumeBlock(void** p, u32 size) {
    void* Ret = (*p);
    (*p) = (u8*)(*p) + size; 
    return Ret;
}

template<typename type>
static inline void
Consume(void** memory) {
    return (type*)ConsumeBlock(memory, sizeof(type));
}


template<typename T>
static inline void
Write(void** p, T item) {
    T* location_as_t = (T*)(*p);
    (*location_as_t) = item;
    (*p) = (u8*)(*p) + sizeof(T);
}

#endif 

