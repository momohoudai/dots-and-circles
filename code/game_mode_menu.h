/* date = August 9th 2021 0:26 pm */

#ifndef GAME_MODE_MENU_H
#define GAME_MODE_MENU_H


struct Game_Mode_Menu_Button {
    AABB2f hitbox;
    b8 is_hovered;
    String text;
    Image_ID image_id;
};

enum Game_Mode_Menu_TransitionType {
    Game_Mode_Menu_TransitionType_Enter,
    Game_Mode_Menu_TransitionType_Exit,
    Game_Mode_Menu_TransitionType_Done,
};

struct Game_Mode_Menu {
    Game_Mode_Menu_Button play_button;
    //Transition transition;
    AudioMixer_Instance* bgm_handle;
    
    Game_Mode_Menu_TransitionType transition_mode;
    Timer transition_enter_timer;
    Timer transition_exit_timer;
    f32 transition_screen_opacity;
    
    Timer beat_timer;
    Timer bg_timer;
};

static inline void
Menu_Free(Permanent_State* perm_state) {
    Game_Mode_Menu* mode = perm_state->menu_mode;
    AudioMixer_Stop(g_mixer, mode->bgm_handle);
}


static inline b8
Menu_Init(Permanent_State* perm_state) 
{
    Game_Mode_Menu* mode = perm_state->menu_mode;
    Arena* mode_arena = &perm_state->mode_arena;
    
    g_platform->show_cursor();
    
    // NOTE(Momo): Transition
    mode->transition_enter_timer = Timer_Create(0.15f);
    mode->transition_exit_timer = Timer_Create(0.15f);
    mode->transition_mode = Game_Mode_Menu_TransitionType_Enter;
    mode->transition_screen_opacity = 1.f; // 1 being completely opaque
    
    //mode->transition = Transition_Create(0.15f);
    
    const f32 button_radius = 45.f;
    const f32 button_y = 200.f;
    
    
    // NOTE(Momo): Play button
    {
        auto* button = &mode->play_button;
        button->hitbox = AABB_Create(Vec_Create(button_radius, button_radius),
                                     Vec_Create(400.f, button_y));
        button->is_hovered = false;
        button->image_id = Image_ID_Play;
        button->text = String_Create("PLAY");
    }
    
    // NOTE(Momo): Play BGM
    mode->bgm_handle = AudioMixer_Play(g_mixer, Sound_ID_MenuBGM, true);
    if (!mode->bgm_handle) {
        return false;
    }
    
    
    // Initialize beat-related timers
    mode->beat_timer = Timer_Create(BeatsPerMinToSecsPerBeat(120.f));
    mode->bg_timer = Timer_Create(BeatsPerMinToSecsPerBeat(120.f));
    
    return true;
}


static inline void
Menu_Update(Permanent_State* perm_state,
            f32 dt) 
{
    Game_Mode_Menu* mode = perm_state->menu_mode;
    
    //- NOTE(Momo): Beat timer/scale
    const f32 ease = EaseInSine(Timer_Percent(&mode->beat_timer));
    f32 beat_scale = Lerp(1.f, 1.1f, ease);
    
    Timer_Tick(&mode->beat_timer, dt);
    if (Timer_IsAtEnd(&mode->beat_timer)) {
        Timer_SetToStart(&mode->beat_timer);
    }
    
    
    
    //- NOTE(Momo): Button checks
    if (mode->transition_mode != Game_Mode_Menu_TransitionType_Done) {
        Vec2f mouse_pos = g_input->design_mouse_pos;
        if (AABB_IsPointWithin(mode->play_button.hitbox, mouse_pos)) {
            mode->play_button.is_hovered = true;
            if(Platform_Input_IsButtonPoked(g_input->button_switch)) {
                mode->transition_mode = Game_Mode_Menu_TransitionType_Exit;
            }
        }
        
        else {
            mode->play_button.is_hovered = false;
        }
    }
    
    
    //- NOTE(Momo): Transition timer update
    switch (mode->transition_mode) {
        case Game_Mode_Menu_TransitionType_Enter: {
            Timer_Tick(&mode->transition_enter_timer, dt);
            mode->transition_screen_opacity = 1.f - Timer_Percent(&mode->transition_enter_timer);
            mode->bgm_handle->volume = Timer_Percent(&mode->transition_enter_timer);
            if (Timer_IsAtEnd(&mode->transition_exit_timer)) {
                mode->transition_mode = Game_Mode_Menu_TransitionType_Done;
            }
            
        } break;
        case Game_Mode_Menu_TransitionType_Exit: {
            Timer_Tick(&mode->transition_exit_timer, dt);
            mode->transition_screen_opacity = Timer_Percent(&mode->transition_exit_timer);
            mode->bgm_handle->volume = 1.f - Timer_Percent(&mode->transition_exit_timer);
            if (Timer_IsAtEnd(&mode->transition_exit_timer)) {
                mode->transition_mode = Game_Mode_Menu_TransitionType_Done;
                perm_state->next_game_mode = Game_Mode_Type_Main;
            }
        } break;
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
    
    //- NOTE(Momo): Title
    Painter_DrawTextCenter(g_painter,
                           Font_ID_Default, 
                           Vec2f{400.f, 600.f}, 
						   Msg_ID_Menu_Dots,
                           95.f * beat_scale);
    
    Painter_DrawTextCenter(g_painter,
                           Font_ID_Default, 
                           Vec2f{400.f, 535.f}, 
						   Msg_ID_Menu_And, 						   
                           60.f * beat_scale);
    
    Painter_DrawTextCenter(g_painter,
                           Font_ID_Default, 
                           {400.f, 445.f}, 
						   Msg_ID_Menu_Circles, 
                           95.f * beat_scale);
    
    //- NOTE(Momo): 'Credits'
    Painter_DrawTextRight(g_painter,
                          Font_ID_Default, 
                          {GAME_DESIGN_WIDTH - 5.f, 5.f}, 
						  Msg_ID_Menu_Credits_1, 
                          28.f);
    
    //- NOTE(Momo): Render Play Button
    {
        auto* button = &mode->play_button;
        Vec2f center = button->hitbox.center;
        Vec2f scale = Vec_Create(AABB_Width(button->hitbox), 
                                 AABB_Height(button->hitbox));
        
        if (button->is_hovered) {
            
            
            Painter_DrawImage(g_painter,
                              button->image_id, 
                              center,
                              scale * beat_scale,
                              C4F_YELLOW);
            
            Painter_DrawTextCenter(g_painter,
                                   Font_ID_Default, 
                                   Vec_Create(center.x, center.y - 85 ), 
                                   button->text, 
                                   35.f * beat_scale,
                                   C4F_YELLOW);
        }
        else {
            Painter_DrawImage(g_painter,
                              button->image_id, 
                              center,
                              scale * beat_scale,
                              C4F_WHITE);
        }
    }
    
    //- NOTE(Momo): Transition fade-in/out render
    Painter_DrawQuad(g_painter,
                     {GAME_DESIGN_WIDTH/2, GAME_DESIGN_HEIGHT/2}, 
                     {GAME_DESIGN_WIDTH, GAME_DESIGN_HEIGHT}, 
                     { 0.f, 0.f, 0.f, mode->transition_screen_opacity });
    
    
}
#endif //GAME_MODE_MENU_H
