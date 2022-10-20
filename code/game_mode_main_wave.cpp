

static inline void
Wave__DrawFromShootDeck(Wave* w, RngSeries* rng) {
    u32 chosen_index = RngSeries_Choice(rng, w->enemy_shoot_type_deck.count); 
    List_PushItem(&w->enemy_shoot_type_hand, w->enemy_shoot_type_deck[chosen_index]);
    g_log("[Wave] Adding new shoot to hand: %d\n", w->enemy_shoot_type_deck[chosen_index]);
    List_Slear(&w->enemy_shoot_type_deck, chosen_index);
}


static inline b8
Wave_Alloc(Wave* w, RngSeries* rng, Arena* a){
    
    if(!List_Alloc(&w->enemy_shoot_type_deck, a, 10)) return false;
    if(!List_Alloc(&w->enemy_shoot_type_hand, a, 10)) return false;
    
	const f32 secs_per_bar = BeatsPerMinToSecsPerBeat(120.f) * 8;
	
    w->spawn_timer = Timer_Create(secs_per_bar);
    w->shoot_draw_timer = Timer_Create(secs_per_bar * 4);
	w->event_timer = Timer_Create(secs_per_bar * 32);
    
    // NOTE(Momo): Enemy travel dist
    w->enemy_travel_dist_timer = Timer_Create(secs_per_bar * 4);
    w->enemy_travel_dist_range = {}; // start from 0.f
    
    // NOTE(Momo): Enemy spawn amount
    w->enemy_spawn_amt_range = { 1, 1 };
    w->enemy_spawn_amt_timer = Timer_Create(secs_per_bar* 4);
    
	List_Clear(&w->enemy_shoot_type_deck);
	List_Clear(&w->enemy_shoot_type_hand);
#if 1
    //List_PushItem(&w->enemy_shoot_type_deck, Enemy_ShootType_Homing); 
    List_PushItem(&w->enemy_shoot_type_deck, Enemy_ShootType_Cross); 
	List_PushItem(&w->enemy_shoot_type_deck, Enemy_ShootType_Plus); 
    List_PushItem(&w->enemy_shoot_type_deck, Enemy_ShootType_Asterisk); 
    List_PushItem(&w->enemy_shoot_type_deck, Enemy_ShootType_SpinCCW); 
    List_PushItem(&w->enemy_shoot_type_deck, Enemy_ShootType_SpinCW); 
	List_PushItem(&w->enemy_shoot_type_deck, Enemy_ShootType_Vertical);
	List_PushItem(&w->enemy_shoot_type_hand, Enemy_ShootType_Horizontal);
	List_PushItem(&w->enemy_shoot_type_deck, Enemy_ShootType_FixedTargetOne);
	//List_PushItem(&w->enemy_shoot_type_hand, Enemy_ShootType_FixedTargetThree);
#else
    List_PushItem(&w->enemy_shoot_type_hand, Enemy_ShootType_FixedTargetOne);
#endif
    
    Wave__DrawFromShootDeck(w, rng);
    return true;
    
}

static inline void
Main__SpawnEnemyWithWaveStats(Game_Mode_Main* mode, Vec2f pos) {
	Wave * w = &mode->wave;
	
	u32 shoot_choice = RngSeries_Choice(&mode->rng, w->enemy_shoot_type_hand.count);
	Enemy_ShootType shoot_type = w->enemy_shoot_type_hand[shoot_choice];
	
	// NOTE(Momo): Mood is 50/50
	MoodType mood = (MoodType)RngSeries_Choice(&mode->rng, MoodType_Count);
	
	// NOTE(Momo): Random travel distance and direction
	// 50/50 between moving and not moving
	b8 moving = RngSeries_CoinFlip(&mode->rng);
	Vec2f target_position = {}; 
	f32 random_travel_dist = {};
	if (moving)
	{
		// TODO: We should find a way not to let the enemies travel off screen
		Vec2f random_direction = RngSeries_Direction2(&mode->rng);
		random_travel_dist = RngSeries_Between(&mode->rng, w->enemy_travel_dist_range);
		target_position = pos + random_direction * random_travel_dist;
		target_position.x = Clamp(target_position.x, 0.f, GAME_DESIGN_WIDTH);
		target_position.y = Clamp(target_position.y, 0.f, GAME_DESIGN_HEIGHT);
	}
	else {
		// static enemy
		target_position = pos;
	}
	
	RngSeries_Between(&mode->rng, w->enemy_travel_dist_range);
	Main__Spawn_Enemy(mode,
					  pos,
					  shoot_type,
					  target_position,
					  mood,
					  0.3f,
					  250.f);
	
	//	g_log("[Wave] Spawned: shoot = %d, moving = %d, travel_dist = %f\n", 
	//  shoot_type, moving, random_travel_dist);
}

static inline void
Main__Update_Wave(Game_Mode_Main* mode, f32 dt) {
    Wave * w = &mode->wave;
	
    //- NOTE(Momo): Draw shoot types from deck
    if (w->enemy_shoot_type_deck.count > 0) {
        if (Timer_IsAtEnd(&w->shoot_draw_timer)) {
            Wave__DrawFromShootDeck(w, &mode->rng);
            Timer_SetToStart(&w->shoot_draw_timer);
        }
        Timer_Tick(&w->shoot_draw_timer, dt);
        
    }
    
	//- NOTE(Momo): 
    if (Timer_IsAtEnd(&w->event_timer)) {
		auto spawn_pattern = (Wave_EventType)RngSeries_Choice(&mode->rng, Wave_EventType_Max);
		
		switch (spawn_pattern) {
			case Wave_EventType_Center: {
				Main__SpawnEnemyWithWaveStats(mode, Vec2f{ 400.f, 400.f });
				Main__SpawnEnemyWithWaveStats(mode, Vec2f{ 400.f, 400.f });
				Main__SpawnEnemyWithWaveStats(mode, Vec2f{ 400.f, 400.f });
				Main__SpawnEnemyWithWaveStats(mode, Vec2f{ 400.f, 400.f });
				
			} break;
			case Wave_EventType_Around: {
				Main__SpawnEnemyWithWaveStats(mode, Vec2f{ 200.f, 200.f });
				Main__SpawnEnemyWithWaveStats(mode, Vec2f{ 200.f, 600.f });
				Main__SpawnEnemyWithWaveStats(mode, Vec2f{ 600.f, 200.f });
				Main__SpawnEnemyWithWaveStats(mode, Vec2f{ 600.f, 600.f });
			} break;
			
		}
		
		g_log("[Wave] Event! %d\n", spawn_pattern);
		Timer_SetToStart(&w->event_timer);
	}
	Timer_Tick(&w->event_timer, dt);
	
    //- NOTE(Momo): Increase the max amount of enemies to spawn at once
    if (w->enemy_spawn_amt_range.max < w->enemy_spawn_amt_max) {
        if (Timer_IsAtEnd(&w->enemy_spawn_amt_timer)) {
            ++w->enemy_spawn_amt_range.max;
            //g_log("[Wave] Enemy max spawn amount increased: %d\n", 
            //      w->enemy_spawn_amt_range.max);
            Timer_SetToStart(&w->enemy_spawn_amt_timer);
        }
        Timer_Tick(&w->enemy_spawn_amt_timer, dt);
        
    }
	
    //- NOTE(Momo):Increase the max travel distance for enemy movement  
    if (w->enemy_travel_dist_range.max < w->enemy_travel_dist_max) {
        if (Timer_IsAtEnd(&w->enemy_travel_dist_timer)) {
            w->enemy_travel_dist_range.max += w->enemy_travel_dist_increment;
            //g_log("[Wave] Enemy max travel dist increased: %f\n", w->enemy_travel_dist_range.max);
            if (w->enemy_travel_dist_range.max > w->enemy_travel_dist_max) {
                w->enemy_travel_dist_range.max = w->enemy_travel_dist_max;
            }
            
            Timer_SetToStart(&w->enemy_travel_dist_timer);
        }
        Timer_Tick(&w->enemy_travel_dist_timer, dt);
    }
    
    //- NOTE(Momo): Spawn from hand
    if(Timer_IsAtEnd(&w->spawn_timer)) {
        // NOTE(Momo): Random the amount of enemies to spawn
        u32 enemies_to_spawn = RngSeries_Between(&mode->rng, w->enemy_spawn_amt_range);
		
		
        //g_log("[Wave] Spawned amount: %d\n", enemies_to_spawn);
		
		
        for(u32 i = 0; i < enemies_to_spawn; ++i) {    
			
			// NOTE(Momo): Random position
			Vec2f pos = {
				RngSeries_Unilateral(&mode->rng) * GAME_DESIGN_WIDTH * 0.9f + 0.05f,
				RngSeries_Unilateral(&mode->rng) * GAME_DESIGN_HEIGHT * 0.9f + 0.05f
			};
			
            Main__SpawnEnemyWithWaveStats(mode, pos);
        }
        Timer_SetToStart(&w->spawn_timer);
    }
    Timer_Tick(&w->spawn_timer, dt);
}
