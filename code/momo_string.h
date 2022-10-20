#ifndef MM_STRING
#define MM_STRING



struct String;
struct String_Split_Result {
    String* items;
    u32 item_count;
};


struct String {
    u8* data;
    u32 count;
	
    inline u8 operator[](u32 index) {
		Assert(index <= count);
		return data[index];
	}
};

struct StringBuffer {
    union {
        String str;
        struct {
            u8* data;
            u32 count;
        };
    };
    u32 capacity;
    
	
    inline u8 operator[](u32 index) {
		Assert(index <= count);
		return data[index];
	}
};

#include "momo_string.cpp"

#endif
