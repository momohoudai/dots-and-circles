#ifndef __GAME_ASSETS_H__
#define __GAME_ASSETS_H__

// NOTE(Momo): Asset types
struct Texture {
    u32 width, height, channels;
    u8* data; // RGBA format
    Renderer_Texture_Handle handle;
};

struct Anime {
    Image_ID* frames;
    u32 frame_count;
};

struct Image {
    Rect2f texture_uv_rect;
    Texture_ID texture_id;
};

struct Font_Glyph {
    Rect2f texture_uv_rect;
    Rect2f glyph_box;
    f32 advance;
};

struct Font {
    // NOTE(Momo): We cater for a fixed set of codepoints. 
    // ASCII 32 to 126 
    // Worry about sparseness next time.
    f32 line_gap;
    f32 ascent;
    f32 descent;
    u32 codepoint_start;
    u32 codepoint_end;
    Texture_ID texture_id;
    
    Font_Glyph* glyphs;
    u32* kernings;
};

struct Sound {
    s16* data;
    u32 data_count;
};

typedef String Msg;

struct Assets {
    Texture* textures;
    u32 texture_count;
    
    Image* images;
    u32 image_count;
    
    Font* fonts;
    u32 font_count;
    
    Anime* animes;
    u32 anime_count;
    
    Msg* msgs;
    u32 msg_count;
    
    Sound* sounds;
    u32 sound_count;
};

// NOTE(Momo): Specialized function for Renderer use
struct TextureInfoForRenderer {
    Renderer_Texture_Handle handle;
    Rect2f uv;
};

#include "game_assets.cpp"
#endif  
