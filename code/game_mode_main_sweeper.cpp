static inline void
Main__Update_Sweeper(Game_Mode_Main* mode, f32 dt) {
	Sweeper *s = &mode->sweeper;
	if (mode->player.combo > GAME_SWEEPER_COMBO_REQUIREMENTS && 
		!s->is_active) 
	{
		
		// Initialize sweeper
		s->top_line = s->btm_line = Line2f {
			Vec2f { -1.f, mode->player.position.y },
			Vec2f { (f32)GAME_DESIGN_WIDTH, mode->player.position.y },
		};
		
		s->is_active = true;
		s->thickness = 5.f;
		mode->player.combo = 0;
		AudioMixer_Play(g_mixer, Sound_ID_Sweeper, false, 0.1f); 
		
	}
	
	// Update sweeper
	if (s->is_active) {
		s->top_line.min.y = s->top_line.max.y += GAME_SWEEPER_SPEED * dt;
		s->btm_line.min.y = s->btm_line.max.y -= GAME_SWEEPER_SPEED * dt;
		f32 line_radius = s->thickness * 0.5f;
		
		auto bullet_lamb = [&](Bullet* b) {
			b8 is_collided = false;
			// NOTE(Momo): It's not very dynamic to do it this way
			// but we don't really care since we make sure that circles 
			// will never move past the sweeper given the sweeper's 
			// thickness and the circle's max speed + radius
			
			Circ2f bullet_circ = Circ_Translate(b->hit_circle, b->position);
			if(b->position.y - b->hit_circle.radius <= s->top_line.min.y + line_radius && 
			   b->position.y + b->hit_circle.radius >= s->top_line.min.y - line_radius ) {
				is_collided = true;
			}
			else if(b->position.y - b->hit_circle.radius <= s->btm_line.min.y + line_radius && 
					b->position.y + b->hit_circle.radius >= s->btm_line.min.y - line_radius ) {
				is_collided = true;
			}
			
			
			if(is_collided)
			{
				BigInt_Add(&mode->score, 50);
				Main__Spawn_Particles(mode, b->position, 5);
				Main__PlayPickupSound(mode, 1.f);
				return true;
			}
			
			return false;
		};
		
		List_ForEachSlearIf(&mode->circle_bullets, bullet_lamb);
		List_ForEachSlearIf(&mode->dot_bullets, bullet_lamb);
		
		
		if (s->btm_line.min.y < 0.f && 
			s->top_line.min.y > GAME_DESIGN_HEIGHT) {
			s->is_active = false;
		}
	}
	
}

static inline void
Main__Render_Sweeper(Game_Mode_Main* mode) {
	Sweeper *s = &mode->sweeper;
	if(s->is_active)
	{
		Painter_DrawLine(g_painter, s->top_line, s->thickness, C4F_GREY5);	
		Painter_DrawLine(g_painter, s->btm_line, s->thickness, C4F_GREY5);
	}
}