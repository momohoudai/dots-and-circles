static inline void
Player_Init(Player* player) {
	player->position = {};
	player->prev_position = {};
	player->size = player->max_size;
	player->hit_circle = Circ2f{ Vec2f{}, 16.f};
	
	// NOTE(Momo): We start as Dot
	player->mood_type = MoodType_Dot;
	player->dot_image_alpha = 1.f;
	player->dot_image_alpha_target = 1.f;
	
	player->dot_image_transition_duration = 0.1f;
	player->dot_image_transition_timer = player->dot_image_transition_duration;
	player->is_dead = false;
	player->combo = 0;
}

static inline void 
Main__Update_Player(Game_Mode_Main* mode, 
                    f32 dt) 
{
    Player* player = &mode->player; 
    player->dot_image_alpha = Lerp(1.f - player->dot_image_alpha_target, 
                                   player->dot_image_alpha_target, 
                                   player->dot_image_transition_timer / player->dot_image_transition_duration);
    
    player->dot_image_transition_timer += dt;
    player->dot_image_transition_timer = 
        Clamp(player->dot_image_transition_timer, 
              0.f, 
              player->dot_image_transition_duration);
    
    
}

static inline void
Main__Render_Score(Game_Mode_Main* mode) {
    static constexpr f32 font_size = 28.f;
    
    
    // NOTE(Momo): Current Score
    {
        f32 x_pos = 10.f;
        f32 y_pos = GAME_DESIGN_HEIGHT - 32.f;
        
        Painter_DrawTextLeft(g_painter,
                             Font_ID_Default, 
                             { x_pos, y_pos },
                             String_Create("CURRENT SCORE"),
                             font_size);
        y_pos -= 30.f;
        
        Painter_DrawBigIntLeft(g_painter,
                               Font_ID_Default,
                               { x_pos, y_pos },
                               &mode->score,
                               font_size);
        
    }
    
    // NOTE(Momo): High Score
    {
        f32 x_pos = GAME_DESIGN_WIDTH - 10.f;
        f32 y_pos = GAME_DESIGN_HEIGHT - 32.f;
        
        Painter_DrawTextRight(g_painter,
                              Font_ID_Default,
                              { x_pos, y_pos },
                              String_Create("HIGH SCORE"),
                              font_size);
        y_pos -= 30.f;
        
        Painter_DrawBigIntRight(g_painter,
                                Font_ID_Default,
                                { x_pos, y_pos },
                                &mode->high_score,
                                font_size);
        
    }
    
}


static inline void 
Main__Render_Player(Game_Mode_Main* mode) 
{
    Player* player = &mode->player;
    
    const f32 ease = EaseInSine(Timer_Percent(&mode->beat_timer));
    f32 beat_scale = Lerp(0.75f, 1.0f, ease);
    
    if (player->is_dead) 
    {
        // do nothing in the end lol
    }
    else {
        Vec2f size = Vec_Create(player->size * beat_scale,
                                player->size * beat_scale);
        {
            c4f color = c4f_Create(1.f, 1.f, 1.f, 1.f - player->dot_image_alpha);
            
            Painter_DrawImage(g_painter,
                              Image_ID_Player_Circle,
                              player->position,
                              size, 
                              color);
        }
        
        
        {
            c4f color = c4f_Create(1.f, 1.f, 1.f, player->dot_image_alpha);
            Painter_DrawImage(g_painter,
                              Image_ID_Player_Dot,
                              player->position,
                              size, 
                              color);
        }
    }
    
}
