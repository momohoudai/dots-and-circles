/* date = May 22nd 2021 7:14 pm */

#ifndef Game_Mode_Type_Main_DEBUG_H
#define Game_Mode_Type_Main_DEBUG_H

static inline void 
Main_RenderDebugLines(Game_Mode_Main* mode){
    Circ2f circle = {};
    circle.origin = mode->player.position;
    circle.radius = mode->player.hit_circle.radius;
    Painter_DrawCircle(g_painter, circle, 1.f, 8, C4F_GREEN);
}

#endif //Game_Mode_Type_Main_DEBUG_H
