/* date = May 22nd 2021 7:09 pm */

#ifndef Game_Mode_Type_Main_COLLISION_H
#define Game_Mode_Type_Main_COLLISION_H


static inline void
Main__Update_PlayerBulletCollision(Game_Mode_Main* mode,
                                   f32 dt)
{
    if (mode->is_invul) {
        return;
    }
    Player* player = &mode->player;
    Circ2f player_circle = Circ_Translate(player->hit_circle, player->prev_position);
    Vec2f player_vel = player->position - player->prev_position;
	b32 play_death_audio_once = false;
    
    auto lamb = [&](Bullet* b) {
        Circ2f b_circle = Circ_Translate(b->hit_circle, b->position);
        Vec2f b_vel = b->direction * b->speed * dt;
        
        if (IsMovingCircleOnMovingCircle(player_circle, 
										 player_vel,
										 b_circle,
										 b_vel)) 
        {
            if (player->mood_type == b->mood_type) {
                Vec2f vector_to_bullet = Vec_Normalize(b->position - player->position);
                Vec2f spawn_pos = player->position + vector_to_bullet * player->hit_circle.radius;
                Main__Spawn_Particles(mode, spawn_pos, 5);
                BigInt_Add(&mode->score, 50);
				Main__PlayPickupSound(mode, 1.f);
				++player->combo;
            }
            else {
                player->is_dead = true;
				if (play_death_audio_once == false) {
					AudioMixer_Play(g_mixer, Sound_ID_Boom, false, 0.15f);
					play_death_audio_once = true;
				}
			}
			return true;
		}
		
		return false;
	};
	List_ForEachSlearIf(&mode->dot_bullets, lamb);
	List_ForEachSlearIf(&mode->circle_bullets, lamb);
}

#endif //Game_Mode_Type_Main_COLLISION_H
