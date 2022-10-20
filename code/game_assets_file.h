#ifndef GAME_ASSETS_TYPES 
#define GAME_ASSETS_TYPES

// NOTE(Momo): For now we cater for a fixed set of codepoints. 
// Namely ASCII 32 to 126. Worry about unicode next time.



enum Texture_ID {
    Texture_ID_Atlas_Default,
    
    Texture_ID_Count,
};


enum Sound_ID {
    Sound_ID_Pickup,
	Sound_ID_Boom,
	Sound_ID_Slowmo,
	Sound_ID_Sweeper,
	
	Sound_ID_MenuBGM,
    Sound_ID_MainBGM1,
    Sound_ID_MainBGM2,
    Sound_ID_MainBGM3,
    Sound_ID_MainBGM4,
    Sound_ID_MainBGM5,
	
    
    Sound_ID_Count,
};

enum Font_ID {
    Font_ID_Default,
    Font_ID_Debug,
    
    Font_ID_Count,
};

enum Anime_ID {
#if 0
    Anime_ID_KaruFont,
#endif
    
    Anime_ID_Count,
};

enum Image_ID {
    Image_ID_Player_Dot,
    Image_ID_Player_Circle,
    Image_ID_Bullet_Dot,
    Image_ID_Bullet_Circle,
    Image_ID_Enemy,
    Image_ID_Particle,
    
    Image_ID_Play,
    
    Image_ID_Count,
};

enum Msg_ID {
	Msg_ID_Menu_Dots,
	Msg_ID_Menu_And,
	Msg_ID_Menu_Circles,
	Msg_ID_Menu_Credits_1,
	Msg_ID_Menu_Credits_2,
	Msg_ID_Main_GameOver,
	Msg_ID_Main_Continue,
    
    Msg_ID_Count,
};

enum Asset_Type {
    Asset_Type_Texture,
    Asset_Type_Image,
    Asset_Type_Font,
    Asset_Type_Sound,
    Asset_Type_Anime,
    Asset_Type_Msg,
};


// NOTE(Momo): asset pack file structures
#pragma pack(push, 1)
struct Asset_File_Entry {
    Asset_Type type;
};

struct Asset_File_Msg {
    Msg_ID id;
    u32 count;
    // NOTE(Momo): Data is:
    // u8 string[count]
};

struct Asset_File_Texture {
    Texture_ID id;
    u32 width;
    u32 height;
    u32 channels;
    // NOTE(Momo): Data is:
    // u8 Pixels[width * height * channels]; 
    
};

struct Asset_File_Image {
    Image_ID id;
    Texture_ID texture_id;
    //Rect2u texture_box;
    Rect2f texture_uv_rect;
};

struct Asset_File_Font_Glyph {
    Rect2f texture_uv_rect;
    u32 codepoint;
    f32 advance;
    Rect2f glyph_box; 
};


// TODO(Momo): Remove codepoint?
struct Asset_File_Font_Kerning {
    s32 kerning;
    u32 codepoint_a;
    u32 codepoint_b;
};


struct Asset_File_Font {
    Font_ID id;
    f32 ascent;
    f32 descent;
    f32 line_gap;
    Texture_ID texture_id; 
    
    u32 codepoint_start;
    u32 codepoint_end;
    
    // NOTE(Momo): Data is:
    // count = codepoint_end - codepoint_start + 1
    // Asset_File_Font_Kerning kernings[count][count]
    // Asset_File_Font_Glyph glyphs[count]
    
};

struct Asset_File_Anime {
    Anime_ID id;
    u32 frame_count;
    // NOTE(Momo): Data is:
    // Image_ID Data[FrameCount]
};

struct Asset_File_Sound {
    Sound_ID id;
    
    u32 data_count;
    // NOTE(Momo): Data is: 
    // s16 Data[DataCount]
};
#pragma pack(pop)

#endif 
