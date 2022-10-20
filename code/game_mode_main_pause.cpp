
#if 0
static inline void
Main_Update_Pause(Game_Mode_Main* mode, f32 dt) {
    if (Platform_Input_IsButtonPoked(g_input->button_pause)) {
        if (mode->is_paused) {
            
            Circ2f player_circle = Circ_Translate(mode->player.hit_circle, mode->player.position);
            Vec2f mouse_pos_in_world = g_input->design_mouse_pos;
            
            if (Bonk2_IsPointWithinCircle(player_circle, mouse_pos_in_world )) {
                g_platform->hide_cursor();
                mode->is_paused = false;
            }
        }
        else {
            g_platform->show_cursor();
            mode->is_paused = true;
        }
    }
    
    if (mode->is_paused) {
        Timer_Tick(&mode->pause_timer, dt);
    }
    else {
        Timer_Untick(&mode->pause_timer, dt);
    }
}


static inline void 
Main__Render_Pause(Game_Mode_Main* mode) {
    if (!mode->is_paused) 
        return; 
    f32 target_alpha = 0.5f;
    f32 percent = Timer_Percent(&mode->pause_timer);
    f32 alpha = Lerp(0.f, 0.5f, percent);
    
    c4f color = c4f_Create(0.f, 0.f, 0.f, alpha);
    
    Painter_DrawQuad(g_painter,
                     Vec_Create((f32)GAME_DESIGN_WIDTH*0.5f, (f32)GAME_DESIGN_HEIGHT*0.5f),
                     Vec_Create((f32)GAME_DESIGN_WIDTH, (f32)GAME_DESIGN_HEIGHT),
                     color);
    
}
#endif // 0