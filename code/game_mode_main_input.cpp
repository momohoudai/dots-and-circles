static inline void
Main__Update_Input(Game_Mode_Main* mode)
{
    Player* player = &mode->player; 
    
    Vec2f direction = {};
    
    player->prev_position = player->position;
	if (g_input->design_mouse_pos.x >= 0.f && 
		g_input->design_mouse_pos.y >= 0.f &&
		g_input->design_mouse_pos.x <= GAME_DESIGN_WIDTH &&
		g_input->design_mouse_pos.y <= GAME_DESIGN_HEIGHT) 
	{
		player->position = g_input->design_mouse_pos;
	}
    
    
    // NOTE(Momo): Absorb mode Switch
    if(Platform_Input_IsButtonPoked(g_input->button_switch)) {
        player->mood_type = 
        (player->mood_type == MoodType_Dot) ? MoodType_Circle : MoodType_Dot;
        
        switch(player->mood_type) {
            case MoodType_Dot: {
                player->dot_image_alpha_target = 1.f;
            } break;
            case MoodType_Circle: {
                player->dot_image_alpha_target = 0.f;
            }break;
            default:{ 
                Assert(false);
            }
        }
        player->dot_image_transition_timer = 0.f;
    }
    
}

