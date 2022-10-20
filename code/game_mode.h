/* date = April 12th 2021 7:00 pm */

#ifndef GAME_MODE_H
#define GAME_MODE_H

enum Game_Mode_Type {
    Game_Mode_Type_Menu,
    Game_Mode_Type_Main,

    
    // special state
    Game_Mode_Type_None,
    Game_Mode_Type_Exit,
    
};


struct Permanent_State {
    b8 is_initialized;
    
    //- NOTE(Momo): Game modes 
    Game_Mode_Type current_game_mode;
    Game_Mode_Type next_game_mode;
    union {
        struct Game_Mode_Menu* menu_mode;
        struct Game_Mode_Main* main_mode;        
    };
    
	
    Assets assets; 
    
    // NOTE(Momo): Arenas
    Arena arena; // main arena
    Arena mode_arena;
    Arena asset_arena;
    Arena frame_arena;
    
    // NOTE(Momo): Audio related
    AudioMixer mixer;

#if INTERNAL    
    // NOTE(Momo): Debug purposes
	f32 game_speed;
	u64 asset_last_modified_time;
	u32 asset_reload_retry_count;
    Inspector inspector;
    Console console;
#endif // INTERNAL

};


#endif //GAME_MODE_H
