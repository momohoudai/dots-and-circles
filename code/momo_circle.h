/* date = June 27th 2021 1:31 pm */

#ifndef MOMO_CIRCLE_H
#define MOMO_CIRCLE_H

template<typename T, u32 N>
struct Circ {
    Vec<T,N> origin;
    T radius;
};

typedef Circ<f32,2> Circ2f;

template<typename T, u32 N>
static inline Circ<T,N>
Circ_Translate(Circ<T,N> lhs, Vec<T,N> v) {
    lhs.origin = lhs.origin + v;
    return lhs;
}

#if 0
//~ NOTE(Momo): Circles
struct circle2f {
    v2f origin;
    f32 radius;
    
};

static inline circle2f
circle2f_Create(v2f origin, f32 radius) {
    return { origin, radius };
}

static inline circle2f
Translate(circle2f lhs, v2f v) {
    lhs.origin = lhs.origin + v;
    return lhs;
}
#endif


#endif //MOMO_CIRCLE_H
