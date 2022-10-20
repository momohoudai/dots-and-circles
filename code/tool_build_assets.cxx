#include <stdio.h>
#include <stdlib.h>
#include "momo.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "game_config.h"
#include "game_assets_file.h"  

#include "tool_build_assets_util.h"
#include "tool_build_assets_atlas.h"

#include "tool_build_assets.h"



static inline Memory_Block
AllocateMemory(u32 size) {
    Memory_Block ret = {};
    ret.data = malloc(size);
    ret.size = size;
    
    return ret;
}

static inline void
FreeMemory(Memory_Block block) {
    free(block.data);
}
int main() {
    printf("Start!\n");
    
    Memory_Block asset_builder_memory = AllocateMemory(Mebibytes(64));
    if (!asset_builder_memory) {
        printf("Failed to initialize asset_builder memory\n");
        return 1; 
    }
    defer { FreeMemory(asset_builder_memory); };
    
    Memory_Block atlas_builder_memory = AllocateMemory(Mebibytes(16));
    if (!atlas_builder_memory) {
        printf("Failed to initialize atlas_builder memory\n");
        return 1; 
    }
    defer { FreeMemory(atlas_builder_memory); };
    
    
    AssetBuilder* asset_builder = Arena_Boot(AssetBuilder, arena, asset_builder_memory.data, asset_builder_memory.size);
    
    AtlasBuilder* atlas_builder = Arena_Boot(AtlasBuilder, arena, atlas_builder_memory.data, atlas_builder_memory.size);
    
    
    
    AtlasBuilder_Begin(atlas_builder, Texture_ID_Atlas_Default, 1024, 1024, 4, 1, 128);
#if 0
    AtlasBuilder_PushImage(atlas_builder, Image_ID_Ryoji, "assets/ryoji.png"); 
    AtlasBuilder_PushImage(atlas_builder, Image_ID_Yuu, "assets/yuu.png"); 
    
    AtlasBuilder_PushImage(atlas_builder, Image_ID_Karu_00, "assets/karu00.png");
    AtlasBuilder_PushImage(atlas_builder, Image_ID_Karu_01, "assets/karu01.png");
    AtlasBuilder_PushImage(atlas_builder, Image_ID_Karu_02, "assets/karu02.png");
    AtlasBuilder_PushImage(atlas_builder, Image_ID_Karu_10, "assets/karu10.png");
    AtlasBuilder_PushImage(atlas_builder, Image_ID_Karu_11, "assets/karu11.png");
    AtlasBuilder_PushImage(atlas_builder, Image_ID_Karu_12, "assets/karu12.png");
    AtlasBuilder_PushImage(atlas_builder, Image_ID_Karu_20, "assets/karu20.png");
    AtlasBuilder_PushImage(atlas_builder, Image_ID_Karu_21, "assets/karu21.png");
    AtlasBuilder_PushImage(atlas_builder, Image_ID_Karu_22, "assets/karu22.png");
    AtlasBuilder_PushImage(atlas_builder, Image_ID_Karu_30, "assets/karu30.png");
    AtlasBuilder_PushImage(atlas_builder, Image_ID_Karu_31, "assets/karu31.png");
    AtlasBuilder_PushImage(atlas_builder, Image_ID_Karu_32, "assets/karu32.png");
#endif
    AtlasBuilder_PushImage(atlas_builder, Image_ID_Player_Dot, "assets/player_white.png");
    AtlasBuilder_PushImage(atlas_builder, Image_ID_Player_Circle, "assets/player_black.png");
    AtlasBuilder_PushImage(atlas_builder, Image_ID_Bullet_Dot, "assets/bullet_dot.png");
    AtlasBuilder_PushImage(atlas_builder, Image_ID_Bullet_Circle, "assets/bullet_circle.png");
    AtlasBuilder_PushImage(atlas_builder, Image_ID_Enemy, "assets/enemy.png");
    AtlasBuilder_PushImage(atlas_builder, Image_ID_Particle, "assets/particle.png");
    AtlasBuilder_PushImage(atlas_builder, Image_ID_Play, "assets/play.png");
    
    AtlasBuilder_PushFont(atlas_builder, Font_ID_Default, "assets/nokiafc22.TTF", 100.f, 32, 90);
    AtlasBuilder_PushFont(atlas_builder, Font_ID_Debug, "assets/DroidSansMono.ttf", 100.f, 32, 126);
    AtlasBuilder_End(atlas_builder, "test.png");
    
    //~ NOTE(Momo): Actual asset building
    printf("[Build Assets] Building Assets Start\n");
    
    printf("[Build Assets] Writing Atlases...\n");
    AssetBuilder_Begin(asset_builder,
                       GAME_ASSET_FILENAME,
                       GAME_ASSET_FILE_SIGNATURE);
    
    AssetBuilder_WriteAtlas(asset_builder, atlas_builder);
    
#if 0
    //- NOTE(Momo): Anime
    printf("[Build Assets] Writing Animes...\n");
    {
        Image_ID front[] = { Image_ID_Karu_30, Image_ID_Karu_31, Image_ID_Karu_32 };
        AssetBuilder_WriteAnime(asset_builder, Anime_ID_KaruFont, front, ArrayCount(front));
    }
#endif
    
    //- NOTE(Momo): Msg
    printf("[Build Assets] Writing Msgs...\n");
    AssetBuilder_WriteMsg(asset_builder, Msg_ID_Menu_Dots, "DOTS");
    AssetBuilder_WriteMsg(asset_builder, Msg_ID_Menu_And, "AND");
    AssetBuilder_WriteMsg(asset_builder, Msg_ID_Menu_Circles, "CIRCLES");
	AssetBuilder_WriteMsg(asset_builder, Msg_ID_Menu_Credits_1, "A GAME BY MOMODEVELOP");
	AssetBuilder_WriteMsg(asset_builder, Msg_ID_Menu_Credits_2, "Twitter: momodevelop");
	AssetBuilder_WriteMsg(asset_builder, Msg_ID_Main_GameOver, "GAME OVER");
	AssetBuilder_WriteMsg(asset_builder, Msg_ID_Main_Continue, "LEFT CLICK TO CONTINUE");
	
	
    //- NOTE(Momo): Sound
    printf("[Build Assets] Writing Sounds...\n");
    AssetBuilder_WriteSound(asset_builder, Sound_ID_Pickup, "assets/pickup.wav");
    AssetBuilder_WriteSound(asset_builder, Sound_ID_Boom, "assets/boom.wav");
	AssetBuilder_WriteSound(asset_builder, Sound_ID_Slowmo, "assets/slowmo_start.wav");
	AssetBuilder_WriteSound(asset_builder, Sound_ID_Sweeper, "assets/sweeper.wav");
	
    AssetBuilder_WriteSound(asset_builder, Sound_ID_MenuBGM, "assets/bgm_menu.wav");
    AssetBuilder_WriteSound(asset_builder, Sound_ID_MainBGM1, "assets/bgm_main_layer_1.wav");
    AssetBuilder_WriteSound(asset_builder, Sound_ID_MainBGM2, "assets/bgm_main_layer_2.wav");
    AssetBuilder_WriteSound(asset_builder, Sound_ID_MainBGM3, "assets/bgm_main_layer_3.wav");
    AssetBuilder_WriteSound(asset_builder, Sound_ID_MainBGM4, "assets/bgm_main_layer_4.wav");
    AssetBuilder_WriteSound(asset_builder, Sound_ID_MainBGM5, "assets/bgm_main_layer_5.wav");
    
    
    AssetBuilder_End(asset_builder);
    printf("[Build Assets] Building Assets End\n");
    
    return 0;
    
}

