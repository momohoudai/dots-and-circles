/* date = June 27th 2021 1:36 pm */

#ifndef MOMO_RAYLINES_H
#define MOMO_RAYLINES_H

template<typename T, u32 N>
struct Line {
    Vec<T,N> min, max;
};
typedef Line<f32,2> Line2f;

struct Ray_Intersect_Times {
    f32 lhs;
    f32 rhs;
};
template<typename T, u32 N>
struct Ray {
    Vec<T,N> origin;
    Vec<T,N> dir;
}; 
typedef Ray<f32,2> Ray2f;

template<typename T, u32 N>
static inline Ray<T,N>
Line_ToRay(Line<T,N> line) {
    Ray<T,N> ret = {};
    ret.origin = line.min;
    ret.dir = line.max - line.min;
    return ret;
}

// TODO(Momo): Make this generic I guess
static inline Ray_Intersect_Times 
Ray_IntersectionTimes(Ray2f lhs, Ray2f rhs) {
    f32 t1;
    f32 t2;
    
    Vec2f p1 = lhs.origin;
    Vec2f p2 = rhs.origin;
    Vec2f v1 = lhs.dir;
    Vec2f v2 = rhs.dir;
    
    t2 = (v1.x*p2.y - v1.x*p1.y - v1.y*p2.x + v1.y*p1.x)/(v1.y*v2.x - v1.x*v2.y);
    t1 = (p2.x + t2*v2.x - p1.x)/v1.x;
    
    return { t1, t2 };
}



#if 0
struct line2f {
    v2f min;
    v2f max;
};


struct ray2f {
    v2f origin;
    v2f dir;
    
    static inline ray2f create(line2f l);
};

struct Ray_Intersect_Times {
    f32 lhs;
    f32 rhs;
};


//~ functions
inline line2f
line2f_Create(v2f min, v2f max) {
    return { min, max };
}

inline line2f
line2f_Create(f32 MinX, f32 MinY, f32 MaxX, f32 MaxY) {
    v2f Min = { MinX, MinY };
    v2f Max = { MaxX, MaxY };
    
    return line2f_Create(Min, Max);
    
}

inline ray2f
ray2f_Create(line2f l) {
    ray2f ret = {};
    ret.origin = l.min;
    ret.dir = l.max -l.min;
    return ret;
}

static inline Ray_Intersect_Times 
ray2f_IntersectionTimes(ray2f lhs, ray2f rhs) {
    f32 t1;
    f32 t2;
    
    v2f p1 = lhs.origin;
    v2f p2 = rhs.origin;
    v2f v1 = lhs.dir;
    v2f v2 = rhs.dir;
    
    
    t2 = (v1.x*p2.y - v1.x*p1.y - v1.y*p2.x + v1.y*p1.x)/(v1.y*v2.x - v1.x*v2.y);
    t1 = (p2.x + t2*v2.x - p1.x)/v1.x;
    
    return { t1, t2 };
}

static inline v2f 
ray2f_GetPoint(ray2f r, f32 time) {
    // O + D * T
    return r.origin + (r.dir * time);
}
#endif 


#endif //MOMO_RAYLINES_H
