#ifndef Game_Mode_Type_Main_CPP
#define Game_Mode_Type_Main_CPP


static inline void
Main__PlayPickupSound(Game_Mode_Main* mode, f32 volume) {
	if (mode->pickup_sound_timer >= 0.1f) {
		AudioMixer_Play(g_mixer, Sound_ID_Pickup, false, volume);
		mode->pickup_sound_timer = 0.f;
	}
	
}


#include "game_mode_main_input.cpp"
#include "game_mode_main_pause.cpp"
#include "game_mode_main_player.cpp"
#include "game_mode_main_bullet.cpp"
#include "game_mode_main_enemy.cpp"
#include "game_mode_main_wave.cpp"
#include "game_mode_main_particle.cpp"
#include "game_mode_main_collision.cpp"
#include "game_mode_main_debug.cpp"
#include "game_mode_main_death_bomb.cpp"
#include "game_mode_main_score_io.cpp"
#include "game_mode_main_sweeper.cpp"

#if INTERNAL
static inline void 
Main_CmdInvul(Console* console, void* context, String args) {
    auto* perm_state = (Permanent_State*)context;
    
    Game_Mode_Main * main_mode = perm_state->main_mode;
    if (main_mode->is_invul) {
        main_mode->is_invul = false;
        String buffer = String_Create("Invul Off");
        Console_PushInfo(console, buffer, C4F_YELLOW);
    }
    else {
        main_mode->is_invul = true;
        String buffer = String_Create("Invul On");
        Console_PushInfo(console, buffer, C4F_YELLOW);
    }
    
    
}
#endif // INTERNAL

static inline void
Main_Free(Permanent_State* perm_state)  
{
    Game_Mode_Main* mode = perm_state->main_mode;
    for (u32 i = 0; i < ArrayCount(mode->bgm_handle); ++i) { 
        AudioMixer_Stop(g_mixer, mode->bgm_handle[i]);
    }
	
#if INTERNAL    
    Console_RemoveCommand(&perm_state->console, 
                          String_Create("invul"));
#endif // INTERNAL
}

static inline b8 
Main_Init(Permanent_State* perm_state) 
{
    Game_Mode_Main* mode = perm_state->main_mode;
    Arena* mode_arena = &perm_state->mode_arena;
    
    g_platform->hide_cursor();
    
    
    if (!List_Alloc(&mode->dot_bullets, mode_arena, GAME_BULLET_CAP/2)) {
        return false;
    }
    
    if (!List_Alloc(&mode->circle_bullets, mode_arena, GAME_BULLET_CAP/2)) {
        return false;
    }
    
    if (!List_Alloc(&mode->enemies, mode_arena, GAME_ENEMY_CAP)) {
        return false;
    }
    
    if (!Queue_Alloc(&mode->particles, mode_arena, GAME_PARTICLE_CAP)) {
        return false;
    }
    
    
    if (!BigInt_Alloc(&mode->score, mode_arena, GAME_SCORE_MAX_PLACES)) {
        return false;
    }
    
    if (!BigInt_Alloc(&mode->high_score, mode_arena, GAME_SCORE_MAX_PLACES)) {
        return false;
    }
    
    // NOTE(Momo): RNG
	u32 seed = (u32)g_platform->get_performance_counter();
    mode->rng = RngSeries_Create(seed); 
    if (!Wave_Alloc(&mode->wave, &mode->rng, mode_arena)) {
        return false;
    }
    
    
    // NOTE(Momo): Player initialization
	Player_Init(&mode->player);
    
    // NOTE(Momo): Read score
    Main__ReadScore(mode);
    
    
    // NOTE(Momo): Game Over related
    mode->game_over_fade_timer = Timer_Create(0.25f);
    mode->game_over_left_click_timer = Timer_Create(1.f);
    
    // NOTE(Momo): Beat timer
    mode->beat_timer = Timer_Create(GAME_SECS_PER_BEAT);
	
    // NOTE(Momo): Transition
    mode->transition_enter_timer = Timer_Create(0.15f);
    mode->transition_exit_timer = Timer_Create(0.15f);
    mode->transition_mode = Game_Mode_Main_TransitionType_Enter;
    mode->transition_screen_opacity = 1.f; // 1 being completely opaque
    
	// NOTE(Momo): Slow-mo
	mode->slowmo_cooldown_timer = Timer_Create(GAME_SLOWMO_COOLDOWN);
	mode->slowmo_resume_timer = Timer_Create(GAME_SLOWMO_RESUME_TRANSITION_DURATION);
	mode->slowmo_effect_timer = Timer_Create(GAME_SLOWMO_EFFECT_DURATION);
	Timer_SetToEnd(&mode->slowmo_effect_timer);
	mode->slowmo_modifier = 1.f; 
	
	// NOTE(Momo): Sweeper
	mode->sweeper.is_active = false;
    
#if INTERNAL
    if (!Console_AddCommand(&perm_state->console, 
                            String_Create("invul"), 
                            Main_CmdInvul, 
							perm_state)) 
	{
        return false;
    }
#endif
    
    // NOTE(Momo): BGM
    {
        const u32 entry_time_offset_beats = 8;
        for (u32 i = 0; i < ArrayCount(mode->bgm_handle); ++i) {
            mode->bgm_handle[i] = AudioMixer_Play(g_mixer, 
												  Sound_ID(Sound_ID_MainBGM1 + i), 
												  true);
            mode->bgm_handle[i]->is_playing = false; 
            mode->bgm_entry_time[i] = (f32)(entry_time_offset_beats * i);
        }
		mode->bgm_timer = 0.f;
    }
    
	// NOTE(Momo): Limit pickup sound
	mode->pickup_sound_this_frame = 0;
	
	// NOTE(Momo): State stuff
    mode->state = Game_Mode_Main_StateType_Spawn;
    mode->is_invul = false;
	
	
    return true; 
    
}


static inline void
Main_NormalState_Update(Permanent_State* perm_state, 
                        f32 dt) 
{
    Game_Mode_Main* mode = perm_state->main_mode;
    
	
	static b8 play_once = false;
	// NOTE(Momo): Whether to activate slowmo
	{
		if (mode->circle_bullets.count + mode->dot_bullets.count >= 
			GAME_SLOWMO_BULLET_LIMIT) 
		{
			// NOTE(Momo): Activate slowmo here
			if (Timer_IsAtEnd(&mode->slowmo_cooldown_timer)) {
				play_once = true;
				mode->slowmo_modifier = 0.f;
				Timer_SetToStart(&mode->slowmo_resume_timer);
				Timer_SetToStart(&mode->slowmo_cooldown_timer);
				Timer_SetToStart(&mode->slowmo_effect_timer);
				AudioMixer_Play(g_mixer, Sound_ID_Slowmo, false, 1.f);
			}
		}
		
		Timer_Tick(&mode->slowmo_cooldown_timer, dt);
	}
	
	// NOTE(Momo): Slowmo update
	if (mode->slowmo_modifier < 1.f) {
		Timer_Tick(&mode->slowmo_resume_timer, dt);
		
		
		mode->slowmo_modifier = EaseOutSine(Timer_Percent(&mode->slowmo_resume_timer));
		if (Timer_IsAtEnd(&mode->slowmo_resume_timer)) {
			mode->slowmo_modifier = 1.f;
		}
		
	}
	
	
	
	Main__Update_Input(mode);
	Main__Update_Player(mode, dt);    
	Main__Update_Bullets(mode, dt);
	Main__Update_Wave(mode, dt);
	Main__Update_Enemies(mode, dt); 
	Main__Update_PlayerBulletCollision(mode, dt);
	Main__Update_Particles(mode, dt);
	Main__Update_Sweeper(mode, dt);
	
	// NOTE(Momo): update pickup sound cooldown
	mode->pickup_sound_timer += dt;
	
	// NOTE(Momo): if player's dead, do dead stuff
	if(mode->player.is_dead) 
	{
		// NOTE(Momo): Drop the death bomb
		Bomb_Drop(&mode->death_bomb, mode->player.position);            
		mode->state = Game_Mode_Main_StateType_PlayerDied;
	}
    
    // NOTE(Momo): Render
    Main__Render_Player(mode);
    Main__Render_Bullets(mode);
    Main__Render_Enemies(mode);
    Main__Render_Particles(mode);
    Main__Render_Score(mode);
	Main__Render_Sweeper(mode);
    
	{
		// Do a flash to indicate slow mo
		if (!Timer_IsAtEnd(&mode->slowmo_effect_timer)) {
			f32 alpha = 1.f - EaseInSine(Timer_Percent(&mode->slowmo_effect_timer));
			Painter_DrawQuad(g_painter, 
							 Vec2f{ GAME_DESIGN_WIDTH*0.5f, GAME_DESIGN_HEIGHT*0.5f },
							 Vec2f{ GAME_DESIGN_WIDTH, GAME_DESIGN_HEIGHT },
							 c4f { 1.f ,1.f, 1.f, alpha * 0.35f} );
			
			Timer_Tick(&mode->slowmo_effect_timer, dt);
		}
	}
	
	
}

static inline void
Main__Render_GameOver(Game_Mode_Main* mode) {
    if (mode->state != Game_Mode_Main_StateType_GameOver) {
        return;
    }
    
	const f32 ease = EaseInSine(Timer_Percent(&mode->beat_timer));
    f32 beat_scale = Lerp(1.f, 1.1f, ease);
    
	
    // Game over
    {    
        f32 alpha = Lerp(0.f, 1.f, Timer_Percent(&mode->game_over_fade_timer));
        Painter_DrawTextCenter(g_painter,
                               Font_ID_Default, 
                               {GAME_DESIGN_WIDTH/2, GAME_DESIGN_HEIGHT/2-40.f}, 
							   Msg_ID_Main_GameOver,
                               75.f * beat_scale,
                               c4f_Create(1.f, 1.f, 1.f, alpha));
    }
    
    
    // left click to continue
    {
        f32 alpha = Lerp(0.f, 1.f, Timer_Percent(&mode->game_over_left_click_timer));
        Painter_DrawTextCenter(g_painter,
                               Font_ID_Default, 
                               { GAME_DESIGN_WIDTH/2, GAME_DESIGN_HEIGHT/2-70.f }, 
                               Msg_ID_Main_Continue, 
                               30.f * beat_scale,
                               c4f_Create(1.f, 1.f, 1.f, alpha));
    }
}

static inline void
Main_GameOverState_Update(Permanent_State* perm_state, 
                          f32 dt) 
{
    // NOTE(Momo): Here, we decide whether to return to main menu etc
    Game_Mode_Main* mode = perm_state->main_mode;
    
    if (mode->score > mode->high_score) {
        BigInt_Set(&mode->high_score, &mode->score);
    }
    
    Timer_Tick(&mode->game_over_fade_timer, dt);
    
    // NOTE(Momo): Fade In/out for 'left click to continue'
    if (mode->is_game_over_fade_in) {
        Timer_Tick(&mode->game_over_left_click_timer, dt);
        if (Timer_IsAtEnd(&mode->game_over_left_click_timer)) {
            mode->is_game_over_fade_in = false;
        }
    }
    else {
        Timer_Untick(&mode->game_over_left_click_timer, dt);
        if (Timer_IsAtStart(&mode->game_over_left_click_timer)) {
            mode->is_game_over_fade_in = true;
        }
    }
    
    if (Platform_Input_IsButtonPoked(g_input->button_switch)) {
        mode->transition_mode = Game_Mode_Main_TransitionType_Exit;
    }
    
    // NOTE(Momo): Render
    //Main_Render_Player(mode);
    //Main_Render_Bullets(mode);
    //Main_Render_Enemies(mode);
    //Main_Render_Particles(mode);
    //Main_Render_Bomb(mode);
    Main__Render_Score(mode);
    Main__Render_GameOver(mode);
    
}

static inline void
Main_PlayerDiedState_Update(Permanent_State* perm_state, 
                            f32 dt) 
{
    // Everything stops
    Game_Mode_Main* mode = perm_state->main_mode;
    
    Main__Update_Input(mode);
    Main__Update_Player(mode, dt);
    Main__Update_Bomb(mode, dt);
    Main__Update_Particles(mode, dt);
    
    // NOTE: PlayerDied -> Spawning state
    // NOTE: Change state if enemy and Bullet count is 0
    if (!mode->death_bomb.is_active)
    {
        mode->state = Game_Mode_Main_StateType_GameOver;
        Timer_SetToStart(&mode->spawn_timer);
        Main__SaveScore(mode);
    }
    
    // NOTE(Momo): Render
    Main__Render_Player(mode);
    Main__Render_Bullets(mode);
    Main__Render_Enemies(mode);
    Main__Render_Particles(mode);
    Main__Render_Bomb(mode);
    Main__Render_Score(mode);
}

static inline void
Main_SpawnState_Update(Permanent_State* perm_state, 
                       f32 dt) 
{
    Game_Mode_Main* mode = perm_state->main_mode;
    
	f32 ease = EaseOutBounce(Clamp(Timer_Percent(&mode->spawn_timer), 0.f, 1.f));
	mode->player.size = mode->player.max_size * ease;
	Main__Update_Input(mode);
	Main__Update_Player(mode, dt);    
	
	// NOTE(Momo): Spawning -> Normal state
	if (Timer_IsAtEnd(&mode->spawn_timer)) {
		mode->state = Game_Mode_Main_StateType_Normal;
		mode->player.size = mode->player.max_size;
	}
	
	Timer_Tick(&mode->spawn_timer, dt);
    
	
    // NOTE(Momo): Render
    Main__Render_Player(mode);
    Main__Render_Score(mode);
    
}

static inline void
Main_Update(Permanent_State* perm_state, 
            f32 dt) 
{
    Game_Mode_Main* mode = perm_state->main_mode;
    
	//- NOTE(Momo): Reset pickup sound counter 
	mode->pickup_sound_this_frame = 0;
	
    //- NOTE(Momo): BGM update 
    {
        for (u32 i = 0; i < ArrayCount(mode->bgm_handle); ++i) {
            if (mode->bgm_handle[i]->is_playing) continue;
            
            if (mode->bgm_entry_time[i] <= mode->bgm_timer) {
                mode->bgm_handle[i]->is_playing = true; 
            }
            
        }
        mode->bgm_timer += dt;
    }
	
	
    //- NOTE(Momo): Background 
    {
        f32 bg_beat_scale = Lerp(1.f, 2.f, EaseInSine(Timer_Percent(&mode->beat_timer)));
        
        const u32 slices = 15;
        const f32 thickness = 3.f * bg_beat_scale;
        
        const c4f color = c4f_Create(0.125f, 0.125f, 0.125f, 0.5f);
        for (u32 i = 0; i < slices + 1; ++i) {
            f32 offset = (f32)GAME_DESIGN_HEIGHT/slices;
            f32 y = offset * i - offset * 0.5f;
            Vec2f start = Vec_Create(0.f, y);
            Vec2f end = Vec_Create((f32)GAME_DESIGN_WIDTH, y);
            Painter_DrawLine(g_painter,
                             Line2f{start, end},
                             thickness,
                             color);
        }
        for (u32 i = 0; i < slices + 1; ++i) {
            f32 offset = (f32)GAME_DESIGN_WIDTH/slices;
            f32 x = offset * i - offset * 0.5f;
            Vec2f start = Vec_Create(x, 0.f);
            Vec2f end = Vec_Create(x, (f32)GAME_DESIGN_HEIGHT);
            Painter_DrawLine(g_painter,
                             Line2f{start, end},
                             thickness,
                             color);
        }
    }
    //- NOTE(Momo): Transition timer update
    switch (mode->transition_mode) {
        case Game_Mode_Main_TransitionType_Enter: {
            Timer_Tick(&mode->transition_enter_timer, dt);
            mode->transition_screen_opacity = 1.f - Timer_Percent(&mode->transition_enter_timer);
            for (u32 i = 0; i < ArrayCount(mode->bgm_handle); ++i) { 
                mode->bgm_handle[i]->volume = Timer_Percent(&mode->transition_enter_timer);
            }
            
            if (Timer_IsAtEnd(&mode->transition_exit_timer)) {
                mode->transition_mode = Game_Mode_Main_TransitionType_Done;
            }
            
        } break;
        case Game_Mode_Main_TransitionType_Exit: {
            Timer_Tick(&mode->transition_exit_timer, dt);
            mode->transition_screen_opacity = Timer_Percent(&mode->transition_exit_timer);
            for (u32 i = 0; i < ArrayCount(mode->bgm_handle); ++i) {
                mode->bgm_handle[i]->volume = 1.f - Timer_Percent(&mode->transition_exit_timer);
            }
            if (Timer_IsAtEnd(&mode->transition_exit_timer)) {
                mode->transition_mode = Game_Mode_Main_TransitionType_Done;
                perm_state->next_game_mode = Game_Mode_Type_Menu;
            }
        } break;
    }
    
    //- NOTE(Momo): State update
    switch(mode->state) {
        case Game_Mode_Main_StateType_Spawn: {
            Main_SpawnState_Update(perm_state, dt);
        } break;
        case Game_Mode_Main_StateType_Normal: {
            Main_NormalState_Update(perm_state, dt);
        }break;
        case Game_Mode_Main_StateType_PlayerDied: {
            Main_PlayerDiedState_Update(perm_state, dt);
        } break;
        case Game_Mode_Main_StateType_GameOver: {
            Main_GameOverState_Update(perm_state, dt);
        } break;
    }
    
#if INTERNAL
    //Main_RenderDebugLines(mode, RenderCommands);
    String buffer = String_Create("Dots: ");
    Inspector_Push(&perm_state->inspector, buffer, mode->dot_bullets.count);
    buffer = String_Create("Circles: ");
    Inspector_Push(&perm_state->inspector, buffer, mode->circle_bullets.count);
    
    buffer = String_Create("Bullets: ");
    Inspector_Push(&perm_state->inspector, buffer, mode->dot_bullets.count + mode->circle_bullets.count);
    
    buffer = String_Create("Enemies: ");
    Inspector_Push(&perm_state->inspector, buffer, mode->enemies.count);
	buffer = String_Create("Combo: ");
    Inspector_Push(&perm_state->inspector, buffer, mode->player.combo);
#endif // INTERNAL
	
    
    // NOTE(Momo): Transition fade-in/out render
    Painter_DrawQuad(g_painter,
                     {GAME_DESIGN_WIDTH/2, GAME_DESIGN_HEIGHT/2}, 
                     {GAME_DESIGN_WIDTH, GAME_DESIGN_HEIGHT}, 
                     { 0.f, 0.f, 0.f, mode->transition_screen_opacity });
    
	
	// NOTE(Momo): Beat update
    Timer_Tick(&mode->beat_timer, dt);
    if (Timer_IsAtEnd(&mode->beat_timer)) {
        Timer_SetToStart(&mode->beat_timer);
    }
}

#endif //GAME_MODE_H











