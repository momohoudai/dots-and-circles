/* date = September 9th 2021 0:22 pm */

#ifndef GAME_MODE_MAIN_PLAYER_H
#define GAME_MODE_MAIN_PLAYER_H

struct Player {
    // NOTE(Momo): Rendering
    f32 dot_image_alpha;
    f32 dot_image_alpha_target;
    f32 dot_image_transition_timer;
    f32 dot_image_transition_duration;
    
    static constexpr f32 max_size = 32.f;
    f32 size;
    
    
    // Collision
    Circ2f hit_circle;
    
    // Physics
    Vec2f position;
    Vec2f prev_position;
    
    // Gameplay
    MoodType mood_type;
    
	u32 combo; 
	
	
    b8 is_dead;
    
};

#endif //GAME_MODE_MAIN_PLAYER_H
