/* date = September 9th 2021 0:21 pm */

#ifndef GAME_MODE_MAIN_BULLET_H
#define GAME_MODE_MAIN_BULLET_H

struct Bullet {
    Vec2f size;
    MoodType mood_type;
    Vec2f direction;
    Vec2f position;
	f32 speed;
    Circ2f hit_circle; 
};

#endif //GAME_MODE_MAIN_BULLET_H
