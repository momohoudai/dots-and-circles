/* date = May 22nd 2021 7:12 pm */

#ifndef Game_Mode_Type_Main_WAVE_H
#define Game_Mode_Type_Main_WAVE_H


enum Wave_EventType  {
	Wave_EventType_Center,
	Wave_EventType_Around,
	Wave_EventType_Max
};

struct Wave {
    List<Enemy_ShootType> enemy_shoot_type_deck;
    List<Enemy_ShootType> enemy_shoot_type_hand;
    
	Timer event_timer;
    Timer spawn_timer;
    Timer shoot_draw_timer;
    
    Timer enemy_travel_dist_timer;
    Range<f32> enemy_travel_dist_range;
    constexpr static f32 enemy_travel_dist_max = 100.f;
    constexpr static f32 enemy_travel_dist_increment = 10.f;
    
    
    Timer enemy_spawn_amt_timer;
    Range<u32> enemy_spawn_amt_range;
    constexpr static u32 enemy_spawn_amt_max = 4;
};



#endif //Game_Mode_Type_Main_WAVE_H
