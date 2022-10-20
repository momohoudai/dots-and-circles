/* date = September 9th 2021 0:22 pm */

#ifndef GAME_MODE_MAIN_PARTICLE_H
#define GAME_MODE_MAIN_PARTICLE_H


struct Particle {
    constexpr static f32 duration = 0.25f;
    constexpr static f32 alpha = 0.8f;
    constexpr static f32 size = 10.f;
    constexpr static f32 min_speed = 10.f;
    constexpr static f32 max_speed = 20.f;
    
    f32 timer;
    Vec2f position;
    Vec2f direction;
    f32 speed;
};

#endif //GAME_MODE_MAIN_PARTICLE_H
