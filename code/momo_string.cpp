

//~ NOTE(Momo): Strings
static inline String
String_Create(u8* buffer, u32 buffer_size) {
    String ret = {};
    ret.data = buffer;
    ret.count = buffer_size;
    
    return ret;
}

// NOTE(Momo): This shouldn't be in release.
// Assumes C-String
static inline String
String_Create(const c8* cstr) {
    String ret = {};
    ret.data = (u8*)cstr;
    ret.count = Sistr_Length(cstr);
    return ret;
    
}
// Assumes C-String
static inline String
String_Create(c8* cstr) {
    String ret = {};
    ret.data = (u8*)cstr;
    ret.count = Sistr_Length(cstr);
    return ret;
    
}

static inline String
String_Create(String src, u32 min, u32 max) {
    return String_Create(src.data + min, max - min);
}

static inline b8
String_IsEqual(String lhs, String rhs) {
    if(lhs.count != rhs.count) {
        return false;
    }
    for (u32 i = 0; i < lhs.count; ++i) {
        if (lhs.data[i] != rhs.data[i]) {
            return false;
        }
    }
    return true;
}


static inline b8
String_IsEqual(String lhs, const char* rhs) {
    for(u32 i = 0; i < lhs.count; ++i) {
        if (lhs.data[i] != rhs[i]) {
            return false;
        }
    }
    return true;
}

static inline b8 
operator==(String lhs, String rhs) {
    return String_IsEqual(lhs, rhs);
}

static inline b8 
operator!=(String lhs, String rhs) {
    return !String_IsEqual(lhs, rhs);
}


static inline b8 
operator==(String lhs, const char* rhs) {
    return String_IsEqual(lhs, rhs);
}

static inline b8 
operator!=(String lhs, const char* rhs) {
    return !String_IsEqual(lhs, rhs);
}

static inline u32
String_Find(String s, u8 item, u32 start_index = 0) {
    for(u32 i = start_index; i < s.count; ++i) {
        if(s.data[i] == item) {
            return i;
        }
    }
    return s.count;
}

static inline String_Split_Result
String_Split(String s, Arena* arena, u8 delimiter) {
    // NOTE(Momo): We are having faith that the arena given is a bump arena.
    // i.e. Strings that are push into the arena will be contiguous 
    // in memory, and thus convertible to an array<String> struct.
    Arena_Marker original_pos = Arena_Mark(arena);
    String_Split_Result ret = {};
    u32 min = 0;
    u32 max = 0;
    
    for (;max != s.count;) {
        max = String_Find(s, delimiter, min);
        
        String* link = Arena_Push<String>(arena);
        if (!link) {
            Arena_Revert(&original_pos);
            return {};
        }
        (*link) = String_Create(s, min, max);
        
        if (ret.items == nullptr) {
            ret.items = link;            
        }
        
        min = max + 1;
        ++ret.item_count;
    }
    return ret;
}

//~ NOTE(Momo): StringBuffer
static inline b8
StringBuffer_Init(StringBuffer* s, u8* buffer, u32 buffer_size) {
    if (!buffer || buffer_size == 0) {
        return false;
    }
    s->data = buffer;
    s->count = 0;
    s->capacity = buffer_size;
    return true;
}


static inline b8
StringBuffer_Alloc(StringBuffer* s, Arena* arena, u32 size) {
    u8* buffer = Arena_PushArray<u8>(arena, size);
    return StringBuffer_Init(s, buffer, size);
}

static inline b8
StringBuffer_Pop(StringBuffer* s) {
    if (s->count <= 0) {
        return false;
    }
    --s->count;
    return true;
}

static inline u32
StringBuffer_Remaining(StringBuffer* s) {
    return s->capacity - s->count;
}

static inline b8
StringBuffer_Copy(StringBuffer* s, String src) {
    if (src.count > s->capacity) {
        return false;
    }
    for (u32 i = 0; i < src.count; ++i ) {
        s->data[i] = src.data[i];
    }
    s->count = src.count;
    return true;
}




static inline b8
StringBuffer_PushU8(StringBuffer* s, u8 item) {
    if (s->count < s->capacity) {
        s->data[s->count++] = item;
        return true;
    }
    return false;
}

static inline b8
StringBuffer_PushString(StringBuffer* s, String src) {
    if (s->count + src.count <= s->capacity) {
        for ( u32 i = 0; i < src.count; ++i ) {
            s->data[s->count++] = src.data[i];
        }
        return true;
    }
    return false;
}

static inline b8
StringBuffer_PushSistr(StringBuffer* s, char* sistr) {
	return StringBuffer_PushString(s, String_Create(sistr));
}

static inline b8
StringBuffer_PushSistr(StringBuffer* s, const char* sistr) {
	return StringBuffer_PushString(s, String_Create(sistr));
}

static inline void 
StringBuffer_Clear(StringBuffer* s) {
    s->count = 0;
}

static inline b8
StringBuffer_PushU32(StringBuffer* s, u32 num) {
    if (num == 0) {
        StringBuffer_PushU8(s, '0');
        return true;
    }
    u32 start_pt = s->count; 
    
    for(; num != 0; num /= 10) {
        s32 digit_to_convert = num % 10;
        b8 success = StringBuffer_PushU8(s, (u8)digit_to_convert + '0');
        if (!success) {
            return false;
        }
    }
    
    // Reverse starting from start point to count
    u32 sub_str_len_half = (s->count - start_pt)/2;
    for(u32 i = 0; i < sub_str_len_half; ++i) {
        Swap(s->data[start_pt + i], s->data[s->count - 1 - i]);
    }
    return true;
}


static inline b8
StringBuffer_PushS32(StringBuffer* s, s32 num) {
    if (num == 0) {
        if(!StringBuffer_PushU8(s, '0')) {
            return false;
        }
        return true;
    }
    
    u32 start_pt = s->count; 
    
    b8 Negative = num < 0;
    num = Abs(num);
    
    for(; num != 0; num /= 10) {
        s32 digit_to_convert = num % 10;
        StringBuffer_PushU8(s, (u8)digit_to_convert + '0');
    }
    
    if (Negative) {
        StringBuffer_PushU8(s, '-');
    }
    
    // Reverse starting from start point to count
    u32 sub_str_len_half = (s->count - start_pt)/2;
    for(u32 i = 0; i < sub_str_len_half; ++i) {
        Swap(s->data[start_pt + i], 
             s->data[s->count-1-i]);
        
    }
    
    return true;
}
static inline void
StringBuffer_PushF32(StringBuffer* s, f32 value, u32 precision) {
	if (value < 0.f) {
		StringBuffer_PushU8(s, '-');	
		value = -value;
	}
	// NOTE(Momo): won't work for values that u32 can't contain
	u32 integer_part = (u32)value;
	StringBuffer_PushU32(s, integer_part);
	StringBuffer_PushU8(s, '.');
	
	value -= (f32)integer_part;
	
	for (u32 i = 0; i < precision; ++i) {
		value *= 10.f;
	}
	
	u32 decimal_part = (u32)value;
	StringBuffer_PushU32(s, decimal_part);
}

// Minimal implementation of sprintf
// %[flags][width][.precision][length]specifier
static inline void
StringBuffer_FormatList(StringBuffer* dest, String format, va_list args) {
	
	u32 at = 0;
    while(at < format.count) {
		
        if (format[at] == '%') {
            ++at;
			
            // TODO(Momo): Parse flags
			// TODO(Momo): Parse width
			// TODO(Momo): Parse precision
			// TODO(Momo): Parse length
            
			switch(format[at]) {
				//- NOTE(Momo): Standard Types
				case 'd': 
				case 'i':{
					s32 value = va_arg(args, s32);
					StringBuffer_PushS32(dest, value);
				} break;
				case 'f': {
					f64 value = va_arg(args, f64);
					StringBuffer_PushF32(dest, (f32)value, 5);
				} break;
				case 's': {
					// c-string
					const char* cstr = va_arg(args, const char*);
					while(cstr[0] != 0) {
						StringBuffer_PushU8(dest, (u8)cstr[0]);
						++cstr;
					}
				} break;
				
				//- NOTE(Momo): Custom types
				case 'S': {
					// String
					String str = va_arg(args, String);
					StringBuffer_PushString(dest, str);
				} break;
				
				default: {
					// death
					Assert(false);
				} break;
			}
			++at;
			
            
        }
        else {
            StringBuffer_PushU8(dest, format[at++]);
        }
		
    }
	
	
}

static inline void 
StringBuffer_Format(StringBuffer* dest, String format, ...) {
    va_list args;
    va_start(args, format);
    StringBuffer_FormatList(dest, format, args);
    va_end(args);
    
}
