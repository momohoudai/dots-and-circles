


static inline void
Main__Spawn_Bullets(Game_Mode_Main* mode, 
                    Vec2f position, 
                    Vec2f direction, 
                    f32 speed, 
                    MoodType mood) 
{
    Bullet* b = nullptr;
    switch (mood) {
        case MoodType_Dot: {
            b = List_Push(&mode->dot_bullets);
        } break;
        case MoodType_Circle: {
            b = List_Push(&mode->circle_bullets);
        } break;
        default: {
            Assert(false);
        }
    }
    b->position = position;
	b->speed = speed;
    b->size = { 16.f, 16.f };
    b->hit_circle = {{0.f, 0.f}, 4.f};
    
    if (Vec_LengthSq(direction) > 0.f) {
	    b->direction = Vec_Normalize(direction);
    }
    b->mood_type = mood;
}

static inline void
Main__Update_Bullets(Game_Mode_Main* mode,
                     f32 dt) 
{
    auto slear_if_lamb = [](Bullet* b, Game_Mode_Main* mode, f32 dt) {
        b->position += b->direction * b->speed * dt * mode->slowmo_modifier;
        
        return b->position.x <= 0.f - b->hit_circle.radius || 
            b->position.x >= GAME_DESIGN_WIDTH + b->hit_circle.radius ||
            b->position.y <= 0.f - b->hit_circle.radius ||
            b->position.y >= GAME_DESIGN_HEIGHT + b->hit_circle.radius;
    };
    
    List_ForEachSlearIf(&mode->dot_bullets, slear_if_lamb, mode, dt);
    List_ForEachSlearIf(&mode->circle_bullets, slear_if_lamb, mode, dt);
    
}

static inline void
Main__Render_Bullets(Game_Mode_Main* mode) 
{
    
    const f32 ease = EaseInSine(Timer_Percent(&mode->beat_timer));
    f32 beat_scale = Lerp(0.8f, 1.f, ease);
    
    auto for_lamb = [&](Bullet* b, Image_ID image) {
        Painter_DrawImage(g_painter,
                          image,
                          b->position,
                          b->size * beat_scale, 
                          C4F_WHITE);
    };
    
    // NOTE(Momo): Circles are in front of Dots and are therefore 'nearer'.
    // Thus we have to render Dots first before Circles.
    //
    List_Foreach(&mode->dot_bullets, for_lamb, Image_ID_Bullet_Dot);
    List_Foreach(&mode->circle_bullets, for_lamb, Image_ID_Bullet_Circle);
    
    
    
}
