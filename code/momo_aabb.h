/* date = June 27th 2021 0:48 pm */

#ifndef MOMO_AABB_H
#define MOMO_AABB_H

template<typename T, u32 N>
struct Rect {
    Vec<T,N> min;
    Vec<T,N> max;
};
typedef Rect<f32,2> Rect2f;
typedef Rect<u32,2> Rect2u;
typedef Rect<s32,2> Rect2s;
typedef Rect<f32,3> Rect3f;

template<typename T, u32 N>
static inline Rect<T,N>
Rect_Translate(Rect<T,N> aabb, Vec<T,N> v) {
    aabb.min += v;
    aabb.max += v;
    return aabb;
    
}


template<typename To, typename From,  u32 N>
static inline Rect<To,N>
Rect_Cast(Rect<From,N> a) {
    Rect<To,N> ret = {};
    
    ret.min = Vec_Cast<To>(a.min);
    ret.max = Vec_Cast<To>(a.max);
    
    return ret;
}

// TODO(Momo): Untested...?
template<typename T, u32 N>
static inline Rect<T,N>
Rect_CreatePosDimsAnchor(Vec<T,N> position, Vec<T,N> dimensions, Vec<T,N> anchor) {
    Rect<T,N> ret = {};
    for (u32 i = 0; i < N; ++i) {
        ret.min[i] = Lerp(0, -dimensions[i], anchor[i]);
        ret.max[i] = Lerp(dimensions[i], 0, anchor[i]);
    }
    return Rect_Translate(ret, position);
}

template<typename T, u32 N>
static inline Rect<T,N>
Rect_Create(Vec<T,N> min, Vec<T,N> max) {
    return { min, max };
}


// WH
template<typename T, u32 N>
static inline Rect<T,N>
Rect_CreateDims(Vec<T,N> dimensions) {
    Rect<T,N> ret = {};
    ret.max = dimensions;
    
    return ret;
}

// XYWH
template<typename T, u32 N>
static inline Rect<T,N>
Rect_CreatePosDims(Vec<T,N> position, Vec<T,N> dimensions) {
    Rect<T,N> ret = {};
    ret.max = dimensions;
    
    return Rect_Translate(ret, position);
}




template<typename T, u32 N>
static inline T
Rect_Dimension(Rect<T,N> a, u32 dimension_index) {
    Assert(dimension_index < N);
    return a.max[dimension_index] - a.min[dimension_index];
}

template<typename T, u32 N>
static inline T
Rect_Width(Rect<T,N> a) {
    return Rect_Dimension(a, 0);
}

template<typename T, u32 N>
static inline T
Rect_Height(Rect<T,N> a) {
    return Rect_Dimension(a, 1);
}

template<typename T, u32 N>
static inline T
Rect_Depth(Rect<T,N> a) {
    return Rect_Dimension(a,2);
}

template<typename T, u32 N>
static inline Vec<T,N>
Rect_GetCenter(Rect<T,N> a) {
    Vec<T,N> ret = {};
    for (u32 i = 0; i < N; ++i) {
        ret[i] = (a.min[i] + a.max[i]) / 2;
    }
    return ret;
}

template<typename T, u32 N>
static inline Rect<T,N>
Rect_Scale(Rect<T,N> lhs, f32 rhs) {
    lhs.min = lhs.min * rhs;
    lhs.max = lhs.max * rhs;
    return lhs;
}

template<typename T, u32 N>
static inline Rect<T,N>
operator*(Rect<T,N> lhs, f32 rhs) {
    return Rect_Scale(lhs, rhs);
}


template<typename T, u32 N>
static inline Rect<T,N>
operator*(f32 lhs, Rect<T,N> rhs) {
    return Rect_Scale(rhs, lhs);
}


template<typename T, u32 N>
static inline b8 
Rect_IsPointWithin(Rect<T,N> aabb, Vec<T,N> pt) {
    for (u32 i = 0; i < N; ++i) {
        if (pt[i] < aabb.min[i] && pt[i] > aabb.max[i]) {
            return false;
        }
    }
    
    return true;
}

// NOTE(Momo): Gets the Normalized values of Aabb A based on another Aabb B
template<u32 N>
static inline Rect<f32,N> 
Rect_Ratio(Rect<f32,N> a, Rect<f32,N> b) {
    Rect<f32,N> ret = {};
    for (u32 i = 0; i < N; ++i) {
        ret.min[i] = Ratio(a.min[i], b.min[i], b.max[i]);
        ret.max[i] = Ratio(a.max[i], b.min[i], b.max[i]);
    }
    return ret;
}

template<typename T, u32 N>
static inline Rect<f32,N>
Rect_Ratio(Rect<T,N> a, Rect<T,N> b) {
    return Rect_Ratio(Rect_Cast<f32>(a), Rect_Cast<f32>(b));
}

template<typename T, u32 N>
static inline b8
Rect_IsValid(Rect<T,N> a){
    for (u32 i = 0; i < N; ++i) {
        if (a.min[i] >= a.max[i]) {
            return false;
        }
    }
    return true;
}

template<typename T, u32 N>
static inline b8 
Rect_IsEqual(Rect<T,N> lhs, Rect<T,N> rhs) {
	for (u32 i = 0; i < N; ++i) {
		if (!IsEqual(lhs.min[i], rhs.min[i])) return false;
		if (!IsEqual(lhs.max[i], rhs.max[i])) return false;
	}
	return true;
}

template<typename T, u32 N>
static inline b8 
operator==(Rect<T,N> lhs, Rect<T,N> rhs) {
	return Rect_IsEqual(lhs, rhs);
}


template<typename T, u32 N>
static inline b8 
operator!=(Rect<T,N> lhs, Rect<T,N> rhs) {
	return !(lhs == rhs);
}


//~ NOTE(Momo): AABB
// AABBs are defined by a center and its radiuses.
template<typename T, u32 N>
struct AABB {
    Vec<T,N> radius;
    Vec<T,N> center;
};
typedef AABB<f32,2> AABB2f;


template<typename T, u32 N>
static inline AABB<T,N>
AABB_Create(Vec<T,N> radius, Vec<T,N> center = {}) {
    return { radius, center };
}

template<typename T, u32 N>
static inline b8 
AABB_IsPointWithin(AABB<T,N> aabb, Vec<T,N> pt) {
    for (u32 i = 0; i < N; ++i) {
        if (pt[i] < (aabb.center[i] - aabb.radius[i]) ||
            pt[i] > (aabb.center[i] + aabb.radius[i])) {
            return false;
        }
    }
    
    return true;
}


template<typename T, u32 N>
static inline T
AABB_Dimension(AABB<T,N> a, u32 dimension_index) {
    Assert(dimension_index < N);
    return a.radius[dimension_index] + a.radius[dimension_index];
}

template<typename T, u32 N>
static inline T
AABB_Width(AABB<T,N> a) {
    return AABB_Dimension(a, 0);
}

template<typename T, u32 N>
static inline T
AABB_Height(AABB<T,N> a) {
    return AABB_Dimension(a, 1);
}

template<typename T, u32 N>
static inline T
AABB_Depth(AABB<T,N> a) {
    return AABB_Dimension(a,2);
}


#endif //MOMO_AABB_H
