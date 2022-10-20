static inline void
Enemy_SetStateSpawn(Enemy* E) {
    E->state_type = Enemy_StateType_Spawning;
    E->state_spawn.timer = 0.f;
}

static inline void
Enemy_SetStateDying(Enemy* E) {
    E->state_type = Enemy_StateType_Dying;
    E->state_dying.timer = 0.f;
}

static inline void
Enemy_SetStateActive(Enemy* E) {
    E->state_type = Enemy_StateType_Active;
    E->state_active.timer = 0.f;
}


static inline void
Enemy_DoStateActive(Enemy* enemy, Player* player, Game_Mode_Main* mode, f32 dt) {
    Enemy_State_Active* active = &enemy->state_active;
    
    
    // Movement
    enemy->position = Lerp(enemy->start_position, 
                           enemy->target_position,
                           active->timer/active->duration);
    
    // Fire
    enemy->shoot_timer += dt * mode->slowmo_modifier;
    if (enemy->shoot_timer > enemy->shoot_rate) {
        switch (enemy->shoot_type) {
            case Enemy_ShootType_Homing: {
                Vec2f direction = Vec_Normalize(player->position - enemy->position);
                Main__Spawn_Bullets(mode, enemy->position, direction, enemy->bullet_speed, enemy->shoot_mood);
            } break;
            case Enemy_ShootType_Asterisk: {
                
                static constexpr Vec2f dirs[] = {
                    { 1.f, 0.f },
                    { 0.f, 1.f },
                    { -1.f, 0.f },
                    { 0.f, -1.f },
                    { 0.7071067f, 0.7071067f },
                    { -0.7071067f, -0.7071067f },
                    { 0.7071067f, -0.7071067f },
                    { -0.7071067f, 0.7071067f },
                };
                
                for (u32 i = 0; i < ArrayCount(dirs); ++i) {
                    Main__Spawn_Bullets(mode, enemy->position, dirs[i], enemy->bullet_speed, enemy->shoot_mood);
                }
                
            } break;
            case Enemy_ShootType_Cross: {
                static constexpr Vec2f dirs[] = {
                    { 0.7071067f, 0.7071067f },
                    { -0.7071067f, -0.7071067f },
                    { 0.7071067f, -0.7071067f },
                    { -0.7071067f, 0.7071067f },
                };
                
                for (u32 i = 0; i < ArrayCount(dirs); ++i) {
                    Main__Spawn_Bullets(mode, enemy->position, dirs[i], enemy->bullet_speed, enemy->shoot_mood);
                }
                
                
            } break;
            case Enemy_ShootType_Plus: {
                static constexpr Vec2f dirs[] = {
                    { 1.f, 0.f },
                    { 0.f, 1.f },
                    { -1.f, 0.f },
                    { 0.f, -1.f },
                };
                
                for (u32 i = 0; i < ArrayCount(dirs); ++i) {
                    Main__Spawn_Bullets(mode, enemy->position, dirs[i], enemy->bullet_speed, enemy->shoot_mood);
                }
                
                
            } break;
            case Enemy_ShootType_Horizontal: {
                static constexpr Vec2f dirs[] = {
                    { 1.f, 0.f },
                    { -1.f, 0.f },
                };
                
                for (u32 i = 0; i < ArrayCount(dirs); ++i) {
                    Main__Spawn_Bullets(mode, enemy->position, dirs[i], enemy->bullet_speed, enemy->shoot_mood);
                }                
                
            } break;
			case Enemy_ShootType_Vertical: {
                static constexpr Vec2f dirs[] = {
                    { 0.f, -1.f },
                    { 0.f, 1.f },
                };
                
                for (u32 i = 0; i < ArrayCount(dirs); ++i) {
                    Main__Spawn_Bullets(mode, enemy->position, dirs[i], enemy->bullet_speed, enemy->shoot_mood);
                }                
                
            } break;
			
            case Enemy_ShootType_SpinCW: {
                Enemy_Shoot_Spin* shoot_spin = &enemy->shoot_spin;
                Main__Spawn_Bullets(mode, 
                                    enemy->position, 
                                    shoot_spin->dirs[shoot_spin->dir_index], 
									enemy->bullet_speed, 
                                    enemy->shoot_mood);
                ++shoot_spin->dir_index;
                if (shoot_spin->dir_index >= ArrayCount(shoot_spin->dirs)) {
                    shoot_spin->dir_index = 0; 
                }
            } break;
            
            case Enemy_ShootType_SpinCCW: {
                Enemy_Shoot_Spin* shoot_spin = &enemy->shoot_spin;
                u32 actual_index = ArrayCount(shoot_spin->dirs) - shoot_spin->dir_index - 1;
                
                Main__Spawn_Bullets(mode, 
                                    enemy->position, 
                                    shoot_spin->dirs[actual_index], 
                                    enemy->bullet_speed, 
                                    enemy->shoot_mood);
                ++shoot_spin->dir_index;
                if (shoot_spin->dir_index >= ArrayCount(shoot_spin->dirs)) {
                    shoot_spin->dir_index = 0; 
                }
            } break;
			case Enemy_ShootType_FixedTargetOne: {
				Vec2f direction = Vec_Normalize(enemy->shoot_fixed_target.lock_on_position - 
												enemy->position);
                Main__Spawn_Bullets(mode, 
									enemy->position, 
									direction, 
									enemy->bullet_speed, 
									enemy->shoot_mood);
			} break;
			case Enemy_ShootType_FixedTargetThree: {
				Vec2f direction = Vec_Normalize(enemy->shoot_fixed_target.lock_on_position - 
												enemy->position);
                Main__Spawn_Bullets(mode, 
									enemy->position, 
									direction, 
									enemy->bullet_speed, 
									enemy->shoot_mood);
			} break;
            default: {
                Assert(false);
            }
        }
        enemy->shoot_timer = 0.f;
    }
    
    // Life time
    active->timer += dt;
    if (active->timer > active->duration) {
        Enemy_SetStateDying(enemy);
    }
}


//~ NOTE(Momo): Main
static inline void
Main__Spawn_Enemy(Game_Mode_Main* mode,
                  Vec2f position,
                  Enemy_ShootType shoot_type, 
                  Vec2f target_position,
                  MoodType shoot_mood,
                  f32 shoot_rate,
                  f32 bullet_speed) 
{
    Enemy* enemy = List_Push(&mode->enemies);
    enemy->position = position;
    
    enemy->shoot_mood = shoot_mood;
    enemy->shoot_timer = 0.f;
    enemy->shoot_rate = shoot_rate;
    enemy->bullet_speed = bullet_speed;
    
    // NOTE(Momo): Special cases
    enemy->shoot_type = shoot_type;
    switch(enemy->shoot_type) {
		case Enemy_ShootType_Homing: 
		case Enemy_ShootType_Cross: 
		case Enemy_ShootType_Plus: {
			//enemy->shoot_rate *= 2.f;
		} break;
		case Enemy_ShootType_Asterisk: {
			//		enemy->shoot_rate *= 4.f;
		} break;
        case Enemy_ShootType_SpinCW: 
        case Enemy_ShootType_SpinCCW: {
            enemy->shoot_spin.dir_index = 
				RngSeries_Choice(&mode->rng, 
								 ArrayCount(enemy->shoot_spin.dirs));
        } break;
		case Enemy_ShootType_FixedTargetOne:
		case Enemy_ShootType_FixedTargetThree: {
			enemy->shoot_fixed_target.lock_on_position = mode->player.position;
		} break;
        
    };
    
    enemy->start_position = position;
    enemy->target_position = target_position;
    
    Enemy_SetStateSpawn(enemy);
}

static inline void 
Main__Update_Enemies(Game_Mode_Main* mode,
                     f32 dt) 
{
    Player* player = &mode->player;
    auto slear_lamb = [&](Enemy* enemy) {
        switch(enemy->state_type) {
            case Enemy_StateType_Spawning: {
                Enemy_State_Spawn* spawn = &enemy->state_spawn;
                spawn->timer += dt;
                
                if (spawn->timer >= spawn->duration) {
                    Enemy_SetStateActive(enemy);
                }
            } break;
            case Enemy_StateType_Active: {
                Enemy_DoStateActive(enemy, player, mode, dt);
            } break;
            case Enemy_StateType_Dying: {
                Enemy_State_Dying* dying = &enemy->state_dying;
                dying->timer += dt;
                
                return dying->timer >= dying->duration;
            } break;
        }
        return false;
    };
    List_ForEachSlearIf(&mode->enemies, slear_lamb);
    
    
}

static inline void
Main__Render_Enemies(Game_Mode_Main* mode) 
{
    u32 current_count = 0;
    
    auto for_lamb = [](Enemy* enemy, Game_Mode_Main* mode){
        f32 size = enemy->size;
        switch(enemy->state_type) {
            case Enemy_StateType_Spawning: {
                Enemy_State_Spawn* spawn = &enemy->state_spawn;
                f32 ease = EaseOutQuad(spawn->timer/spawn->duration);
                size = enemy->size * ease;
            } break;
            case Enemy_StateType_Dying: {
                Enemy_State_Dying* Dying = &enemy->state_dying;
                f32 ease = 1.f - Dying->timer/Dying->duration;
                size = enemy->size * ease;
            } break;
            case Enemy_StateType_Active: {
                const f32 ease = EaseInSine(Timer_Percent(&mode->beat_timer));
                f32 beat_scale = Lerp(0.75f, 1.f, ease);
                size = enemy->size * beat_scale;
            } break;
        }
        Painter_DrawImage(g_painter,
                          Image_ID_Enemy,
                          enemy->position,
                          Vec2f{size, size},
                          0.f , 
                          C4F_WHITE);
        
    };
    
    List_Foreach(&mode->enemies, for_lamb, mode);
}
