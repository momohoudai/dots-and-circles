


static inline void 
Bomb_Drop(Bomb* death_bomb, Vec2f position) {
    // NOTE(Momo): Drop the death bomb
    death_bomb->is_active = true;
    death_bomb->radius = 0.f;
    death_bomb->position = position;
}

static inline void
Bomb_Update(Bomb* death_bomb, Game_Mode_Main* mode, f32 dt) {
    if (!death_bomb->is_active) {
        return;
    }
    death_bomb->radius += GAME_DEATH_BOMB_SPEED * dt;
    
    Circ2f death_bomb_circle = Circ2f { death_bomb->position, death_bomb->radius };
    
    auto bullet_lamb = [&](Bullet* b) {
        Circ2f bullet_circ = Circ_Translate(b->hit_circle, b->position);
        
        // NOTE(Momo): We can safely assume that the circles are not moving
        if (IsCircleOnCircle(death_bomb_circle,
                                  bullet_circ)) 
        {
            Vec2f vector_to_bullet = Vec_Normalize(b->position - death_bomb->position);
            Vec2f spawn_pos = death_bomb->position + vector_to_bullet * death_bomb->radius;
            BigInt_Add(&mode->score, 10);
            Main__Spawn_Particles(mode, spawn_pos, 5);
			Main__PlayPickupSound(mode, 1.f);
            return true;
        }
        
        return false;
    };
    
    List_ForEachSlearIf(&mode->circle_bullets, bullet_lamb);
    List_ForEachSlearIf(&mode->dot_bullets, bullet_lamb);
    
    auto enemy_lamb = [&](Enemy* e) {
        Circ2f enemy_circ = Circ2f{ e->position, 0.1f };
        
        // NOTE(Momo): We can safely assume that the circles are not moving
        if (IsCircleOnCircle(death_bomb_circle, enemy_circ)) 
        {
            Vec2f vector_to_bullet = Vec_Normalize(e->position - death_bomb->position);
            Vec2f spawn_pos = death_bomb->position + vector_to_bullet * death_bomb->radius;
            Main__Spawn_Particles(mode, spawn_pos, 5);
			Main__PlayPickupSound(mode, 1.f);
            return true;
        }
        
        return false;
    };
    List_ForEachSlearIf(&mode->enemies, enemy_lamb);
    if (death_bomb->radius >= GAME_DESIGN_WIDTH * 2.f)  {
        death_bomb->is_active = false;
    }
}

static inline void 
Bomb_Render(Bomb* death_bomb) {
    if (!death_bomb->is_active) return;
    
    Painter_DrawCircle(g_painter,
                       Circ2f{ death_bomb->position, death_bomb->radius },
                       5.f, 32, C4F_WHITE);
}

static inline void
Main__Update_Bomb(Game_Mode_Main* mode, f32 dt) {
    Bomb_Update(&mode->death_bomb, mode, dt);
    //Bomb_Update(&mode->fever_bomb, mode, dt);
}

static inline void
Main__Render_Bomb(Game_Mode_Main* mode)
{
    Bomb_Render(&mode->death_bomb);
    //Bomb_Render(&mode->fever_bomb);
}

