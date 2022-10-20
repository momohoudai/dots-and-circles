static inline void
Main__Spawn_Particles(Game_Mode_Main* mode, 
                      Vec2f position,
                      u32 amount) 
{
    for (u32 I = 0; I < amount; ++I) {
        Particle* p = Queue_Push(&mode->particles);
        if (!p) {
            return;
        }
        p->position = position;
        p->direction.x = RngSeries_Bilateral(&mode->rng);
        p->direction.y = RngSeries_Bilateral(&mode->rng);
        p->direction = Vec_Normalize(p->direction);
        p->timer = 0.f;
        p->speed = RngSeries_Between(&mode->rng, p->min_speed, p->max_speed);
    }
}

static inline void 
Main__Update_Particles(Game_Mode_Main* mode, f32 dt) {
    Queue<Particle>* q = &mode->particles;
    
    auto pop_lamb = [](Particle* p) {
        return p->timer >= p->duration;
    };
    Queue_PopUntil(q, pop_lamb);
    
    
    auto for_lamb = [](Particle* p, f32 dt) {
        p->timer += dt;
        p->position += p->direction * p->speed * dt;
    };
    Queue_ForEach(q, for_lamb, dt);
}


static inline void
Main__Render_Particles(Game_Mode_Main* mode) {
    Queue<Particle>* q = &mode->particles;
    
    auto for_each_lamb = [&](Particle* p) {
        f32 ease = 1.f - (p->timer/p->duration);
        f32 alpha = p->alpha * ease;
        f32 size = p->size * ease;
        
        Painter_DrawImage(g_painter,
                          Image_ID_Particle,
                          p->position,
                          {size, size},
                          c4f_Create(1.f, 1.f, 1.f, alpha));
        
    };
    Queue_ForEach(q, for_each_lamb);
}
