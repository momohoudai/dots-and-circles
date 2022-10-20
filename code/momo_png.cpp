#define PNG_DEBUG 0

#if PNG_DEBUG
#include <stdio.h>
#define PNG__Log(...) printf(__VA_ARGS__)
#else
#define PNG__Log
#endif


struct PNG__Context {
    Stream stream;
    Arena* arena; 
    
    Stream image_stream;
    u32 image_width;
    u32 image_height;
    u32 image_channels;
    
    
    Stream unfiltered_image_stream; // for filtering and deflating
	
    
    // other useful info
    u32 bit_depth;
};


struct PNG__Header {
    u8 signature[8];
};

// 5.3 Chunk layout
// | length | type | data | CRC
struct PNG__Chunk_Header {
    u32 length;
    union {
        u32 type_u32;
        u8 type[4];
    };
};


#pragma pack(push, 1)
struct PNG__Chunk_Data_IHDR {
    u32 width;
    u32 height;
    u8 bit_depth;
    u8 colour_type;
    u8 compression_method;
    u8 filter_method;
    u8 interlace_method;
};
#pragma pack(pop)

struct PNG__Chunk_Footer {
    u32 crc; 
};

// ZLIB header notes:
// Bytes[0]:
// - compression flags bit 0-3: Compression Method (CM)
// - compression flags bit 4-7: Compression Info (CINFO)
// Bytes[1]:
// - additional flags bit 0-4: FCHECK 
// - additional flags bit 5: Preset dictionary (FDICT)
// - additional flags bit 6-7: Compression level (FLEVEL)
struct PNG__IDAT_Header {
    u8 compression_flags;
    u8 additional_flags;
};


struct PNG__Huffman {
	// NOTE(Momo): Canonical ordered symbols
    u16* symbols; 
    u32 symbol_count;
    
	// NOTE(Momo): Number of symbols per length
	// i.e. code_lengths[1] is the number of symbols with length 1.
    u16* lengths;
    u32 length_count;
};

// NOTE(Momo): Modified from Annex D of PNG specification:
// https://www.w3.org/TR/2003/REC-PNG-20031110/#D-CRCAppendix
// crc variable indicates the starting register
static inline u32
PNG__CalculateCRC32(u8* data, u32 data_size) {
	static constexpr u32 crc_table[256] =
	{
		0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
		0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
		0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
		0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
		0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
		0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
		0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
		0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
		0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
		0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
		0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
		0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
		0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
		0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
		0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
		0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
		0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
		0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
		0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
		0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
		0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
		0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
		0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
		0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
		0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
		0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
		0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
		0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
		0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
		0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
		0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
		0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d,
	};
	
	u32 r = 0xFFFFFFFFL;
	for (u32 i = 0; i < data_size; ++i) {
		r = crc_table[(r ^ data[i]) & 0xFF] ^ (r >> 8);
	}
	return r ^ 0xFFFFFFFFL;
}

static inline s32
PNG__Huffman_Decode(Stream* src_stream, PNG__Huffman huffman) {
    s32 code = 0;
    s32 first = 0;
    s32 index = 0;
    
    for (u32 len = 1; len <= huffman.length_count - 1; ++len) {
        code |= Stream_ConsumeBits(src_stream, 1);
        s32 count = huffman.lengths[len];
        if(code - count < first) {
			return huffman.symbols[index + (code - first)];
		}
        index += count;
        first += count;
        first <<= 1;
        code <<= 1;
    }
    
    return -1;
}

// NOTE(Momo): 
// https://datatracker.ietf.org/doc/html/rfc1951
// Section 3.2.2
// TODO(Momo): We might want to think about extracting this 
static inline void
PNG__Huffman_Compute(PNG__Huffman* h,
					 Arena* arena, 
					 u16* codes,
					 u32 codes_size, 
					 u32 max_lengths) 
{
    PNG__Huffman ret = {};
    
	// NOTE(Momo): Each code corresponds to a symbol
    h->symbol_count = codes_size;
    h->symbols = Arena_PushArray<u16>(arena, codes_size);
    
	
	// NOTE(Momo): We add +1 because lengths[0] is not possible
	// TODO(Momo): We can optimize this a bit by always treating
	// length[0] as length 1.
    h->length_count = max_lengths;
    h->lengths = Arena_PushArray<u16>(arena, max_lengths + 1);
    
    // 1. Count the number of codes for each code length
    for (u32 sym = 0; sym < codes_size; ++sym)  {
        u16 len = codes[sym];
        ++h->lengths[len];
    }
    
    // 2. Numerical value of smallest code for each code length
    Arena_Marker temp_mark = Arena_Mark(arena);
    u16* len_offset_table = Arena_PushArray<u16>(arena, max_lengths);
    defer { Arena_Revert(&temp_mark); };
    
    for (u32 len = 1; len < max_lengths-1; ++len) {
        len_offset_table[len+1] = len_offset_table[len] + h->lengths[len]; 
    }
    
    
    // 3. Assign numerical values to all codes
    for (u32 sym = 0; sym < codes_size; ++sym)
    {
        u16 len = codes[sym];
        if (len > 0) {
            u16 code = len_offset_table[len]++;
            h->symbols[code] = (u16)sym;
        }
    }
    
}


static inline b8
PNG__Deflate(Stream* src_stream, Stream* dest_stream, Arena* arena) 
{
    
    static constexpr u16 lens[29] = { /* Size base for length codes 257..285 */
        3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
        35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
    static constexpr u16 len_ex_bits[29] = { /* Extra bits for length codes 257..285 */
        0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
        3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
    static constexpr u16 dists[30] = { /* Offset base for distance codes 0..29 */
        1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
        257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
        8193, 12289, 16385, 24577};
    static constexpr u16 dist_ex_bits[30] = { /* Extra bits for distance codes 0..29 */
        0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
        7, 7, 8, 8, 9, 9, 10, 10, 11, 11,
        12, 12, 13, 13 };
    
    
    u8 BFINAL = 0;
    while(BFINAL == 0){
        Arena_Marker scratch = Arena_Mark(arena);
        defer { Arena_Revert(&scratch); };
        
        BFINAL = (u8)Stream_ConsumeBits(src_stream, 1);
        u16 BTYPE = (u8)Stream_ConsumeBits(src_stream, 2);
        PNG__Log(">>> BFINAL: %d\n", BFINAL);
        PNG__Log(">>> BTYPE: %d\n", BTYPE);
        switch(BTYPE) {
            case 0b00: {
				Stream_FlushBits(src_stream);
				
                PNG__Log(">>>> No compression\n");
                Stream_ConsumeBits(src_stream, 5);
                u16 LEN = (u16)Stream_ConsumeBits(src_stream, 16);
                u16 NLEN = (u16)Stream_ConsumeBits(src_stream, 16);
                PNG__Log(">>>>> LEN: %d\n", LEN);
                PNG__Log(">>>>> NLEN: %d\n", NLEN);
                if ((u16)LEN != ~((u16)(NLEN))) {
                    PNG__Log("LEN vs NLEN mismatch!\n");
                    return false;
                }
                // TODO: complete this
            } break;
            case 0b01: 
            case 0b10: {
                PNG__Huffman lit_huffman = {};
                PNG__Huffman dist_huffman = {};
                
                if (BTYPE == 0b01) {
                    // Fixed huffman
                    PNG__Log(">>>> Fixed huffman\n");
					
                    u16 lit_codes[288] = {};
                    u16 dist_codes[32] = {};
                    
					// TODO(Momo): This is kinda fixed, so we
					// can probably cache it?
                    u32 lit = 0;
                    for (; lit < 144; ++lit) {
						lit_codes[lit] = 8;
					}
                    for (; lit < 256; ++lit) {
						lit_codes[lit] = 9;
					}
                    for (; lit < 280; ++lit) {
						lit_codes[lit] = 7;
					}
                    for (; lit < ArrayCount(lit_codes); ++lit) {
						lit_codes[lit] = 8;
					}
                    for (lit = 0; lit < ArrayCount(dist_codes); ++lit) {
						dist_codes[lit] = 5;
					}
                    
					// TODO(Momo): max symbols is same as codes_size??
					// Can we do something about it?
                    PNG__Huffman_Compute(&lit_huffman,
										 arena, 
										 lit_codes, 
										 ArrayCount(lit_codes),
										 15);
                    PNG__Huffman_Compute(&dist_huffman,
										 arena,
										 dist_codes,
										 ArrayCount(dist_codes),
										 15);
                    
                }
                else // BTYPE == 0b10
                {
                    // TODO: Dynamic huffman
					PNG__Log(">>>> Dynamic huffman\n");
					u32 HLIT = Stream_ConsumeBits(src_stream, 5) + 257;
					u32 HDIST = Stream_ConsumeBits(src_stream, 5) + 1;
					u32 HCLEN = Stream_ConsumeBits(src_stream, 4) + 4;
					PNG__Log(">>>>> HLIT: %d\n", HLIT);
					PNG__Log(">>>>> HDIST: %d\n", HDIST);
					PNG__Log(">>>>> HCLEN: %d\n", HCLEN);
					
					static constexpr u32 order[] = {
						16, 17, 18, 0, 8 ,7, 9, 6, 10, 5, 
						11, 4, 12, 3, 13, 2, 14, 1, 15,
					};
					
					u16 code_codes[19] = {};
					
					for(u32 i = 0; i < HCLEN; ++i) {
						code_codes[order[i]] = (u16)Stream_ConsumeBits(src_stream, 3);
					}
					
					PNG__Huffman code_huffman = {};
					PNG__Huffman_Compute(&code_huffman,
										 arena,
										 code_codes,
										 ArrayCount(code_codes),
										 7);
					
					
					u16* lit_dist_codes = Arena_PushArray<u16>(scratch, HDIST + HLIT);
					
					// NOTE(Momo): Decode
					// Loop until end of block code recognize
					u32 last_len = 0;
					for(u32 i = 0; i < (HDIST + HLIT);) {
						
						s32 sym = PNG__Huffman_Decode(src_stream, code_huffman);
						
						if(sym >= 0 && sym <= 15) {
							lit_dist_codes[i++] = (u16)sym;
						}
						else 
						{	
							u32 times_to_repeat = 0;
							u16 code_to_repeat = 0;
							if (sym == 16) {
								// Copy the previous code length 3-6 times
								if (i == 0) return false;
								
								times_to_repeat = 3 + Stream_ConsumeBits(src_stream, 2);
								code_to_repeat = lit_dist_codes[i-1];
								
							}
							
							else if (sym == 17) {
								// Repeat a code length of 0 for 3-10 times
								times_to_repeat = 3 + Stream_ConsumeBits(src_stream, 3);
							}
							else if (sym == 18) {
								// Repeat a code length of 0 for 11-138 times
								times_to_repeat = 11 + Stream_ConsumeBits(src_stream, 7);
							}
							else {
								// Invalid symbol
								return false;
							}
							
							while(times_to_repeat--) {
								lit_dist_codes[i++] = code_to_repeat;
							}
							
						}
						
					}
					
					PNG__Huffman_Compute(&lit_huffman,
										 arena, 
										 lit_dist_codes, 
										 HLIT,
										 15);
                    PNG__Huffman_Compute(&dist_huffman,
										 arena,
										 lit_dist_codes + HLIT,
										 HDIST,
										 15);					
                }
				
				// NOTE(Momo): Actual decoding
                for (;;) 
                {
                    s32 sym = PNG__Huffman_Decode(src_stream, lit_huffman);
                    // NOTE(Momo): Normal case
                    if (sym <= 255) { 
                        u8 byte_to_write = (u8)(sym & 0xFF); 
                        Stream_Write<u8>(dest_stream, byte_to_write);
                    }
                    // NOTE(Momo): Extra code case
                    else if (sym >= 257) {
                        sym -= 257;
                        if (sym >= 29) {
                            return false;
                        }
                        u32 len = lens[sym] + Stream_ConsumeBits(src_stream, len_ex_bits[sym]);
                        sym = PNG__Huffman_Decode(src_stream, dist_huffman);
                        if (sym < 0) {
                            return false;
                        }
                        u32 dist = dists[sym] + Stream_ConsumeBits(src_stream, dist_ex_bits[sym]);
                        while(len--) {
                            umi target_index = dest_stream->current - dist;
                            u8 byte_to_write = dest_stream->data[target_index];
                            Stream_Write<u8>(dest_stream, byte_to_write);
                        }
                    }
                    else { 
                        // sym == 256
                        break;
                    }
                }
                PNG__Log("\n");
            } break;
            default: {
                PNG__Log("Error\n");
                return false;
            }
        }
    }
    return true;
}





static inline u32 
PNG__GetImageSize(PNG__Context* ctx) {
    return ctx->image_width * ctx->image_height * ctx->image_channels;
}

static inline u32 
PNG__GetChannelsFromColourType(u32 colour_type) {
    // NOTE(Momo): Determine the channels
    // colour_type 1 = Pallete used
    // colour_type 2 = Colour used 
    // colour_type 4 = alpha used
    switch(colour_type){
        case 0: {
            return 1; // Grayscale
        } break;
        case 2: {
            return 3; // RGB
        } break;
        case 3: {
            // TODO: we do something special here?
            // Maybe return 0 and then outside check if 0
            // then try to determine from palette?
            return 0;
        } break;
        case 4: {
            return 2; // Grayscale + alpha
        } break;
        case 6: { 
            return 4; // RGBA
        } break;
        default: {
            Assert(false);
            return 0;
        }
    }
}

static inline b8 
PNG__IsFormatSupported(PNG__Chunk_Data_IHDR* IHDR){
    if ((IHDR->colour_type != 6 || IHDR->colour_type == 2) &&
        IHDR->bit_depth != 8 &&
        IHDR->compression_method == 0 &&
        IHDR->filter_method != 0 &&
        IHDR->interlace_method != 0) 
    {
        
        return false;
    }
    return true;
}

static inline b8
PNG__IsSignatureValid(u8* comparee) {
	static constexpr u8 signature[] = { 
		137, 80, 78, 71, 13, 10, 26, 10 
	};
	
    for (u32 i = 0; i < ArrayCount(signature); ++i) {
        if (signature[i] != comparee[i]) {
            return false;
        }
    }
    
    return true;
}

//~ NOTE(Momo): Filtering
static inline b8
PNG__Filter_None(PNG__Context* c) {
    for (u32 i = 0; i < c->image_width * c->image_channels; ++i ){
        u8* pixel_byte = Stream_Consume<u8>(&c->unfiltered_image_stream);
        if (pixel_byte == nullptr) {
			return false;
        }
        PNG__Log("%02X ", (u32)(*pixel_byte));
        Stream_Write<u8>(&c->image_stream, *pixel_byte);
    }
	PNG__Log("\n");
    return true;
}

static inline b8
PNG__Filter_Sub(PNG__Context* c) {
    u32 bpp = (c->image_channels * c->bit_depth)/8;
    for (u32 i = 0; i < c->image_width * c->image_channels; ++i ){
        
        u8* pixel_byte_p = Stream_Consume<u8>(&c->unfiltered_image_stream);
        if (pixel_byte_p == nullptr) {
            return false;
        }
        u8 pixel_byte = (*pixel_byte_p); // Sub(x)
        if (i < bpp) {
            PNG__Log("%02X ", (u32)pixel_byte);
            Stream_Write<u8>(&c->image_stream, pixel_byte);
        }
        else {
            u32 current_index = c->image_stream.current;
            u8 left_reference = c->image_stream.data[current_index - bpp]; // Raw(x-bpp)
            u8 pixel_byte_to_write = (pixel_byte + left_reference) % 256;  
            
            PNG__Log("%02X ", (u32)pixel_byte_to_write);
            Stream_Write<u8>(&c->image_stream, pixel_byte_to_write);
        }
        
    }    
	PNG__Log("\n");
	
    return true;
}

static inline b8
PNG__Filter_Average(PNG__Context* c) {
    u32 bpp = (c->image_channels * c->bit_depth)/8;
    u32 scanline_bytes = c->image_width * c->image_channels * (c->bit_depth/8);
    
    for (u32 i = 0; i < c->image_width * c->image_channels; ++i ){
        
        u8* pixel_byte_p = Stream_Consume<u8>(&c->unfiltered_image_stream);
        if (pixel_byte_p == nullptr) {
            return false;
        }
        u8 pixel_byte = (*pixel_byte_p); // Sub(x)
        if (i < bpp || c->image_stream.current < scanline_bytes ) {
            PNG__Log("%02X ", (u32)pixel_byte);
            Stream_Write<u8>(&c->image_stream, pixel_byte);
        }
        else {
            u32 current_index = c->image_stream.current;
            u8 left = c->image_stream.data[current_index - bpp]; // Raw(x-bpp)
            u8 top = c->image_stream.data[current_index - scanline_bytes]; // Prior(x)
            
            // NOTE(Momo): Formula uses floor((left+top)/2). 
            // Integer Truncation should do the job!
            u8 pixel_byte_to_write = (pixel_byte + (left + top)/2) % 256;  
            
            PNG__Log("%02X ", (u32)pixel_byte_to_write);
            Stream_Write<u8>(&c->image_stream, pixel_byte_to_write);
        }
        
    }
	PNG__Log("\n");
	
    return true;
}

static inline b8
PNG__Filter_Paeth(PNG__Context* c) {
    // TODO
    return false;
}

static inline b8
PNG__Filter_Up(PNG__Context* c) {
    u32 scanline_bytes = c->image_width * c->image_channels * (c->bit_depth/8);
    for (u32 i = 0; i < c->image_width * c->image_channels; ++i ){
        u8* pixel_byte_p = Stream_Consume<u8>(&c->unfiltered_image_stream);
        if (pixel_byte_p == nullptr) {
            return false;
        }
        u8 pixel_byte = (*pixel_byte_p); // Up(x)
        
        // NOTE(Momo): Ignore first scanline
        if (c->image_stream.current < scanline_bytes) {
            PNG__Log("%02X ", (u32)pixel_byte);
            Stream_Write<u8>(&c->image_stream, pixel_byte);
        }
        else {
            u32 current_index = c->image_stream.current;
            u8 top = c->image_stream.data[current_index - scanline_bytes]; 
            u8 pixel_byte_to_write = (pixel_byte + top) % 256;  
            
            PNG__Log("%02X ", (u32)pixel_byte_to_write);
            Stream_Write<u8>(&c->image_stream, pixel_byte_to_write);
        }
    }
	PNG__Log("\n");
	
    return true;
}


static inline b8
PNG__Filter(PNG__Context* c) {
	
    Stream_Reset(&c->unfiltered_image_stream);
	
    // NOTE(Momo): Filter
    // data always starts with 1 byte indicating the type of filter
    // followed by the rest of the chunk.
    while(!Stream_IsEos(&c->unfiltered_image_stream)) {
        u8* filter_type_p = Stream_Consume<u8>(&c->unfiltered_image_stream);
        u8 filter_type = (*filter_type_p);
		
        // NOTE(Momo): https://www.w3.org/TR/PNG-Filters.html
        PNG__Log("Filter Type %d: ", (u32)(filter_type));
        switch(filter_type) {
            case 0: { // None
                if (!PNG__Filter_None(c)) return false;
            } break;
            case 1: { // Sub
                if (!PNG__Filter_Sub(c)) return false;
            } break;
            case 2: {
                if (!PNG__Filter_Up(c)) return false;
            } break;
            case 3: {
                if (!PNG__Filter_Average(c)) return false;
            } break;
            case 4: {
                if (!PNG__Filter_Paeth(c)) return false;
            } break;
            default: {
                return false;
            };
        };
    }
    return true;
    
}

//~ NOTE(Momo): Chunk processing
static inline b8
PNG__ProcessChunk_IHDR(PNG__Context* c) {
	Stream stream = c->stream; 
	auto* IHDR = Stream_Consume<PNG__Chunk_Data_IHDR>(&stream);
    
    // NOTE(Momo): Width and height is in Big Endian
    // We assume that we are currently in a Little Endian system
    IHDR->width = EndianSwapU32(IHDR->width);
    IHDR->height = EndianSwapU32(IHDR->height);
    
    PNG__Log("IHDR: \nwidth: %d\nheight: %d\nbit_depth: %d\ncolour_type: %d\ncompression_method: %d\nfilter_method: %d\ninterlace_method: %d\n\n",
             IHDR->width, IHDR->height, IHDR->bit_depth, IHDR->colour_type, IHDR->compression_method, IHDR->filter_method, IHDR->interlace_method);
    
    if (!PNG__IsFormatSupported(IHDR)) {
        return false;
	}
    
    c->image_width = IHDR->width;
    c->image_height = IHDR->height;
    c->image_channels = PNG__GetChannelsFromColourType(IHDR->colour_type);
    c->bit_depth = IHDR->bit_depth;
    
    // NOTE(Momo): For reserving memory for image
    if(!Stream_Alloc(&c->image_stream, c->arena, PNG__GetImageSize(c))){
		return false;
	}
    
    // NOTE(Momo): Allow space for unfiltered image. 
    // One extra byte per row for filter 'type'
    if (!Stream_Alloc(&c->unfiltered_image_stream, 
                      c->arena, c->image_width * c->image_height *  c->image_channels + c->image_height))
    {
        return false;
    }
	return true;
}

static inline b8
PNG__ProcessChunk_IDAT(PNG__Context* c) {
    Stream idat_stream = c->stream; 
    
    u32 CM = Stream_ConsumeBits(&idat_stream, 4);
    u32 CINFO = Stream_ConsumeBits(&idat_stream, 4);
    u32 FCHECK = Stream_ConsumeBits(&idat_stream, 5); //not needed?
    u32 FDICT = Stream_ConsumeBits(&idat_stream, 1);
    u32 FLEVEL = Stream_ConsumeBits(&idat_stream, 2); //useless?
    
    PNG__Log(">> CM: %d\n>> CINFO: %d\n>> FCHECK: %d\n>> FDICT: %d\n>>FLEVEL: %d\n",
             CM, 
             CINFO,
             FCHECK, 
             FDICT, 
             FLEVEL); 
	
    if (CM != 8 || FDICT != 0 || CINFO > 7) {
        return false;
    }
    
    return PNG__Deflate(&idat_stream, &c->unfiltered_image_stream, c->arena);
}

static inline b8
PNG__ProcessChunk_IEND(PNG__Context* c) {
	
	PNG__Log("Ended\n");
	for (u32 i = 0; i < c->unfiltered_image_stream.data_size; ++i) {
		PNG__Log("%02X ", c->unfiltered_image_stream.data[i]);	
	}
	PNG__Log("\n");
	
	return PNG__Filter(c);
    
}


// NOTE(Momo): For the code here, we are going to assume that 
// the PNG file we are reading is correct. i.e. we don't emphasize on 
// checking correctness of the PNG outside of the most basic of checks (e.g. sig)
//
static inline PNG
PNG_Read(Memory_Block png_memory,
		 Arena* arena) 
{
    PNG__Context ctx = {};
    ctx.arena = arena;
    Stream_Init(&ctx.stream, png_memory.data, png_memory.size);
    
    auto* png_header = Stream_Consume<PNG__Header>(&ctx.stream);  
    if (!PNG__IsSignatureValid(png_header->signature)) {
        return {};
	}
    
    while(!Stream_IsEos(&ctx.stream)) {
        auto* chunk_header = Stream_Consume<PNG__Chunk_Header>(&ctx.stream);
        chunk_header->length = EndianSwapU32(chunk_header->length);
        switch(chunk_header->type_u32) {
			case FourCC("IHDR"): {
				if(!PNG__ProcessChunk_IHDR(&ctx)) {
					return {};
				}
			} break;
            case FourCC("IDAT"): {
				if(!PNG__ProcessChunk_IDAT(&ctx)) {
					return {};
				}
            } break;
            case FourCC("IEND"): {            
                if(!PNG__ProcessChunk_IEND(&ctx)) {					
					return {};
				}
				else {	
					PNG ret = {};
					ret.width = ctx.image_width;
					ret.height = ctx.image_height;
					ret.channels = ctx.image_channels;
					ret.data = ctx.image_stream.data;
					return ret;
				}
            } break;
            default: {
                // NOTE(Momo): For now, we don't care about the rest of the chunks
                PNG__Log("Ignoring chunk: %c%c%c%c\n", 
                         chunk_header->type[0], 
                         chunk_header->type[1], 
                         chunk_header->type[2], 
                         chunk_header->type[3]); 
            };
        }
        Stream_ConsumeBlock(&ctx.stream, chunk_header->length);
        Stream_Consume<PNG__Chunk_Footer>(&ctx.stream);
    }
	
    return {};
	
}


// TODO(Momo): bits per pixel?
// TODO(Momo): Maybe we shift this to another file? momo_png_write.h?
// NOTE(Momo): Really dumb way to write.
// Just have a IHDR, IEND and a single IDAT that's not encoded lul
static inline Memory_Block
PNG_Write(PNG image, Arena* arena) {
	Assert(image.width > 0 && image.height > 0 && image.channels == 4 && image.data != 0);
	
    const u32 image_size = image.width * image.height * image.channels;
	const u32 idat_size = (sizeof(PNG__Chunk_Header) +
						   sizeof(PNG__Chunk_Footer) +
						   sizeof(u8)*3 + sizeof(u16) + image_size);
	
	
	const u32 expected_memory_required = (sizeof(PNG__Chunk_Header) * 2 + // IHDR + IEND headers
										  sizeof(PNG__Chunk_Footer) * 2 + // IHDR + IEND footers
										  sizeof(PNG__Chunk_Data_IHDR) + 
										  idat_size * 1);
	
	
	Stream stream = {};
    if (!Stream_Alloc(&stream, arena, expected_memory_required)) {
		return {};
	}
	
	// NOTE(Momo): Header
	static constexpr u8 signature[] = { 
		137, 80, 78, 71, 13, 10, 26, 10 
	};
	Stream_WriteBlock(&stream, (void*)signature, sizeof(signature));
	
	
	// NOTE(Momo): Write IHDR
	{
		u8* crc_data_start = nullptr;
		u32 crc_data_size = 0;
		
		PNG__Chunk_Header header = {};
		header.type_u32 = FourCC("IHDR");
		header.length = sizeof(PNG__Chunk_Data_IHDR);
		header.length = EndianSwapU32(header.length);
		Stream_Write<PNG__Chunk_Header>(&stream, header);
		crc_data_size += sizeof(PNG__Chunk_Header) - 4;
		crc_data_start = stream.data + stream.current - 4;
		
		PNG__Chunk_Data_IHDR IHDR = {};
		IHDR.width = image.width;
		IHDR.height = image.height;
		IHDR.bit_depth = 8; // ??
		IHDR.colour_type = 4;
		IHDR.compression_method = 0;
		IHDR.filter_method = 0;
		IHDR.interlace_method = 0;
		Stream_Write<PNG__Chunk_Data_IHDR>(&stream, IHDR);
		crc_data_size += sizeof(PNG__Chunk_Data_IHDR);
		
		PNG__Chunk_Footer footer = {};
		footer.crc = PNG__CalculateCRC32(crc_data_start, crc_data_size); 
		footer.crc = EndianSwapU32(footer.crc);
		Stream_Write<PNG__Chunk_Footer>(&stream, footer);
		
	}
	
	// TODO(Momo): Write IDAT
	{
		// TODO(Momo): Split into smaller chunks that u16 can fit
		u16 data_size = (u16)(image.width * image.height * image.channels);
		
		PNG__Chunk_Header header = {};
		header.type_u32 = FourCC("IDAT");
		header.length = sizeof(3 + data_size); 
		Stream_Write<PNG__Chunk_Header>(&stream, header);
		
		// NOTE(Momo): Hardcoded IDAT chunk header header that fits our use-case
		//
		// CM = 8
		// CINFO = any number < 7? 1?
		// FCHECK = 23? if CM == 8 and CINFO == 1
		// FDIC = 0;
		// FLEVEL = 1? Documentation says it doesn't matter;
		u8 compression_method = 8 | 1 << 4;
		u8 compression_flag = 23 | 1 << 6;
		Stream_Write<u8>(&stream, compression_method);
		Stream_Write<u8>(&stream, compression_flag);
		
		
		// NOTE(Momo): Deflate chunk header
		// Let's just have one big ass chunk.
		//
		// BFINAL = 1 (1 bit); // indicates if it's the final block
		// BTYPE = 0 (2 bits); // indicates no compression
		// 
		// 
		u8 compressed_block_header = 1;
		Stream_Write<u8>(&stream, compressed_block_header);
		
		// TODO(Momo): LEN has a max value, so we need to seperate the data
		u16 LEN = data_size; // number of data bytes in the block
		u16 NLEN = ~LEN; // one's complement of LEN
		Stream_Write<u16>(&stream, LEN);
		Stream_Write<u16>(&stream, NLEN);
		
		// NOTE(Momo): Just output the whole data here
		Stream_WriteBlock(&stream, image.data, LEN);
		
		
		PNG__Chunk_Footer footer = {};
		footer.crc = 0; // ??
		Stream_Write<PNG__Chunk_Footer>(&stream, footer);
	}
	
	// TODO(Momo): Write IEND
	{
		PNG__Chunk_Header header = {};
		header.type_u32 = FourCC("IEND");
		header.length = 0;
		Stream_Write<PNG__Chunk_Header>(&stream, header);
		
		PNG__Chunk_Footer footer = {};
		footer.crc = 0; // ??
		Stream_Write<PNG__Chunk_Footer>(&stream, footer);
	}
	
	return { stream.data, stream.current };
	
}
