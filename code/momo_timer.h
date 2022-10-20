#ifndef __MOMO_TIMER__
#define __MOMO_TIMER__


struct Timer {
    f32 current;
    f32 end;
};

static constexpr Timer
Timer_Create(f32 duration) {
    //Assert(!IsEqual(duration, 0.f));
    Timer ret = {};
    ret.end = duration;
    return ret;
}

static inline void
Timer_Tick(Timer* t, f32 dt) {
    t->current += dt;
    if (t->current >= t->end) {
        t->current = t->end;
    }
}

static inline void
Timer_Untick(Timer* t, f32 dt) {
    t->current -= dt;
    if (t->current < 0.f ) {
        t->current = 0.f;
    }
}

static inline void
Timer_SetToStart(Timer* t) {
    t->current = 0.f;
}

static inline void
Timer_SetToEnd(Timer* t) {
	t->current = t->end;
}


static inline b8
Timer_IsAtEnd(Timer* t) {
    return t->current >= t->end;
}

static inline b8
Timer_IsAtStart(Timer* t) {
    return t->current <= 0.f;
}

// Returns [0 - 1]
static inline f32
Timer_Percent(Timer* t) {
    return t->current / t->end;
}

#endif
