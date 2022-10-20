#ifndef Game_Mode_Type_Main_H
#define Game_Mode_Type_Main_H


enum MoodType {
    MoodType_Dot,
    MoodType_Circle,
    
    MoodType_Count,
};

enum Game_Mode_Main_TransitionType {
    Game_Mode_Main_TransitionType_Enter,
    Game_Mode_Main_TransitionType_Exit,
    Game_Mode_Main_TransitionType_Done,
};

#include "game_mode_main_player.h"
#include "game_mode_main_particle.h"
#include "game_mode_main_bullet.h"
#include "game_mode_main_enemy.h"
#include "game_mode_main_wave.h"
#include "game_mode_main_bomb.h"
#include "game_mode_main_sweeper.h"

enum Game_Mode_Main_StateType {
    Game_Mode_Main_StateType_Spawn,
    Game_Mode_Main_StateType_Normal,
    Game_Mode_Main_StateType_PlayerDied,
    Game_Mode_Main_StateType_GameOver,
};



struct Game_Mode_Main {
    Game_Mode_Main_StateType state;
    b8 is_game_over_fade_in;
    
    //Transition transition;
    Timer spawn_timer;
    Timer game_over_left_click_timer;
    Timer game_over_fade_timer;
    
    // Beat
    Timer beat_timer;
	
	// Slow-mo
	b8 is_slowmo_active;
	Timer slowmo_cooldown_timer;
	Timer slowmo_resume_timer;
	Timer slowmo_effect_timer;
	Timer slowmo_to_play_resume_sound_timer;
	f32 slowmo_modifier;
    
    Game_Mode_Main_TransitionType transition_mode;
    Timer transition_enter_timer;
    Timer transition_exit_timer;
    f32 transition_screen_opacity;
    
    Player player;
    
    List<Bullet> circle_bullets;
    List<Bullet> dot_bullets;
    List<Enemy> enemies;
    Queue<Particle> particles;
    Bomb death_bomb;
	Sweeper sweeper;
    
	
    Wave wave;
    RngSeries rng;
    BigInt score;
    BigInt high_score;
    
    // Audio handles
    AudioMixer_Instance* bgm_handle[5];
    f32 bgm_entry_time[ArrayCount(bgm_handle)];
    f32 bgm_timer;
    
    b8 is_invul;
	
	f32 pickup_sound_timer;
	u32 pickup_sound_this_frame;
};

#endif //GAME_MODE_H
