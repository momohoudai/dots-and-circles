/* date = May 22nd 2021 7:08 pm */

#ifndef Game_Mode_Type_Main_ENEMY_H
#define Game_Mode_Type_Main_ENEMY_H


//~ NOTE(Momo): Enemy States
enum Enemy_StateType {
    Enemy_StateType_Spawning,
    Enemy_StateType_Active,
    Enemy_StateType_Dying
        
};
struct Enemy_State_Spawn {
    f32 timer;
    constexpr static f32 duration = 0.5f;
};

struct Enemy_State_Dying {
    f32 timer;
    constexpr static f32 duration = 0.5f;
};

struct Enemy_State_Active {
    f32 timer;
    constexpr static f32 duration = 10.f;
};


//~ NOTE(Momo): Enemy shoot patterns

enum Enemy_ShootType  {
    Enemy_ShootType_Homing,
    
    // static types
    Enemy_ShootType_Cross, // x
    Enemy_ShootType_Plus, // +
    Enemy_ShootType_Asterisk, // *
	Enemy_ShootType_Horizontal, // -
	Enemy_ShootType_Vertical, // |
	
	// shoot at fixed target based on 
	// player position on when spawned
	Enemy_ShootType_FixedTargetOne,
	Enemy_ShootType_FixedTargetThree,
    
    // spinning types 
    Enemy_ShootType_SpinCW,
    Enemy_ShootType_SpinCCW,
    
    Enemy_ShootType_Count,
};


struct Enemy_Shoot_Spin {
    static constexpr Vec2f dirs[] = {
        { 0.f, 1.f },
        { 0.7071067f, 0.7071067f },
        
        { 1.f, 0.f },
        { 0.7071067f, -0.7071067f },
        
        { 0.f, -1.f },
        { -0.7071067f, -0.7071067f },
        
        { -1.f, 0.f },
        { -0.7071067f, 0.7071067f },
        
    };
    u32 dir_index;
};


struct Enemy_Shoot_FixedTarget {
	Vec2f lock_on_position;
};

//~ 
struct Enemy {
    constexpr static f32 size = 32.f; 
    
    Vec2f position;
    
    //- NOTE(Momo): State machine
    Enemy_StateType state_type;
    union {
        Enemy_State_Spawn state_spawn;
        Enemy_State_Active state_active;
        Enemy_State_Dying state_dying;
    };
    
    //- NOTE(Momo): Movement related
    Vec2f target_position;
    Vec2f start_position;
    
    
    //- NOTE(Momo): Shoot-related
    Enemy_ShootType shoot_type;
    union {
        // Additional information required by other shoot types
        Enemy_Shoot_Spin shoot_spin;
		Enemy_Shoot_FixedTarget shoot_fixed_target;
    };
    f32 shoot_timer;
    f32 shoot_rate;
    MoodType shoot_mood;
    f32 bullet_speed;
    
    
};

#endif 