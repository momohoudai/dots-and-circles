/* date = June 27th 2021 0:22 pm */

#ifndef MOMO_VECTOR_H
#define MOMO_VECTOR_H

// NOTE(Momo): Vectors
// If you want to create, just directly use
// initialization list, ie. { a, b, c };
template<typename T, u32 N>
struct Vec {
    T e[N];
    
    inline auto& operator[](u32 i) { 
        Assert(i < N); 
        return e[i]; 
    }
};

template<typename To, typename From,  u32 N>
static inline Vec<To,N>
Vec_Cast(Vec<From,N> a) {
    Vec<To,N> ret = {};
    for (u32 i = 0; i < N; ++i) {
        ret[i] = (To)(a[i]);
    }
    return ret;
}

template<typename T, u32 N> 
static inline Vec<T,N>
Vec_Add(Vec<T,N> lhs, Vec<T,N> rhs) {
    for (u32 i = 0; i < N; ++i) {
        lhs[i] += rhs[i];
    }
    return lhs;
}

template<typename T, u32 N>
static inline Vec<T,N>
operator+(Vec<T,N> lhs, Vec<T,N> rhs) {
    return Vec_Add(lhs, rhs);
}


template<typename T, u32 N>
static inline Vec<T,N>&
operator+=(Vec<T,N>& lhs, Vec<T,N> rhs) {
    return lhs = lhs + rhs;
}

template<typename T, u32 N> 
static inline Vec<T,N>
Vec_Sub(Vec<T,N> lhs, Vec<T,N> rhs) {
    for (u32 i = 0; i < N; ++i) {
        lhs[i] -= rhs[i];
    }
    return lhs;
}

template<typename T, u32 N>
static inline Vec<T,N>
operator-(Vec<T,N> lhs, Vec<T,N> rhs) {
    return Vec_Sub(lhs, rhs);
}

template<typename T, u32 N>
static inline Vec<T,N>&
operator-=(Vec<T,N>& lhs, Vec<T,N> rhs) {
    return lhs = lhs - rhs;
}

template<typename T, u32 N>
static inline Vec<T,N>
Vec_Mul(Vec<T,N> lhs, T rhs) {
    for (u32 i = 0; i < N; ++i) {
        lhs[i] *= rhs;
    }
    return lhs;
}

template<typename T, u32 N>
static inline Vec<T,N>
Vec_Mul(Vec<T,N> lhs, Vec<T,N> rhs) {
    for (u32 i = 0; i < N; ++i) {
        lhs[i] *= rhs[i];
    }
    return lhs;
}

template<typename T, u32 N>
static inline Vec<T,N> 
operator*(Vec<T,N> lhs, T rhs) {
    return Vec_Mul(lhs, rhs);
}


template<typename T, u32 N>
static inline Vec<T,N> 
operator*=(Vec<T,N>& lhs, T rhs) {
    return lhs = lhs * rhs;
}

template<typename T, u32 N>
static inline Vec<T,N> 
operator*(Vec<T,N> lhs, Vec<T,N> rhs) {
    return Vec_Mul(lhs, rhs);
}


template<typename T, u32 N>
static inline Vec<T,N> 
operator*=(Vec<T,N>& lhs, Vec<T,N> rhs) {
    return lhs = lhs * rhs;
}

template<typename T, u32 N>
static inline Vec<T,N> 
operator*(T lhs, Vec<T,N> rhs) {
    return Vec_Mul(rhs, lhs);
}


template<typename T, u32 N>
static inline Vec<T,N> 
Vec_Div(Vec<T,N> lhs, T rhs) {
    for (u32 i = 0; i < N; ++i) {
        lhs[i] /= rhs;
    }
    return lhs;
}

template<typename T, u32 N>
static inline Vec<T,N> 
operator/(Vec<T,N> lhs, T rhs) {
    return Vec_Div(lhs, rhs);
}


template<typename T, u32 N>
static inline Vec<T,N> 
operator/=(Vec<T,N>& lhs, T rhs) {
    return lhs = lhs / rhs;
}


template<typename T, u32 N>
static inline Vec<T,N>
Vec_Ratio(Vec<T,N> l, Vec<T,N> r) {
    for (u32 i = 0; i < N; ++i) {
        l[i] /= r[i];
    }
    return l;
}

template<typename T, u32 N>
static inline Vec<T,N>
Vec_Negate(Vec<T,N> v){
    for (u32 i = 0; i < N; ++i)  {
        v[i] = -v[i];
    }
    return v;
}

template<typename T, u32 N>
static inline Vec<T,N>
operator-(Vec<T,N> v) {
    return Negate(v);
}

template<typename T, u32 N>
static inline b8 
Vec_IsEqual(Vec<T,N> l, Vec<T,N> r) {
    for (u32 i = 0; i < N; ++i) {
        if (!IsEqual(l[i], r[i])) {
            return false;
        }
    }
    return true;
}

template<typename T, u32 N>
static inline b8
operator==(Vec<T,N> l, Vec<T,N> r) {
    return Vec_IsEqual(l,r);
}

template<typename T, u32 N>
static inline b8
operator!=(Vec<T,N> l, Vec<T,N> r) {
    return !(l == r);
}


template<typename T, u32 N>
static inline f32 
Vec_Dot(Vec<T,N> l, Vec<T,N> r) {
    T ret = {};
    for (u32 i = 0; i < N; ++i) {
        ret += l[i] * r[i];
    }
    return ret;
}

template<typename T, u32 N>
static inline Vec<T,N> 
Vec_Midpoint(Vec<T,N> l, Vec<T,N> r)  {
    return (l + r)/T(2); 
}

template<typename T, u32 N>
static inline T
Vec_LengthSq(Vec<T,N> v) { 
    // NOTE(Momo): Dot Product trick!
    return Vec_Dot(v, v);
}


template<typename T, u32 N>
static inline T 
Vec_Length(Vec<T,N> l)  { 
    return Sqrt(Vec_LengthSq(l));
}

template<typename T, u32 N>
static inline T
Vec_DistanceSq(Vec<T,N> l, Vec<T,N> r) {
    return Vec_LengthSq(r - l);
}

template<typename T, u32 N>
static inline T
Vec_Distance(Vec<T,N> l, Vec<T,N> r)  { 
    return Sqrt(Vec_DistanceSq(l, r)); 
}

template<typename T, u32 N>
static inline Vec<T,N> 
Vec_Normalize(Vec<T,N> v)  {
    T len = Vec_Length(v);
    Vec<T,N> ret = v/len;
    return ret;
}

template<typename T, u32 N>
static inline T
Vec_AngleBetween(Vec<T,N> l, Vec<T,N> r) {
    T l_len = Vec_Length(l);
    T r_len = Vec_Length(r);
    T lr_dot = Vec_Dot(l,r);
    T ret = Acos(lr_dot/(l_len * r_len));
    return ret;
}

template<typename T, u32 N>
static inline Vec<T,N> 
Vec_Project(Vec<T,N> from, Vec<T,N> to) { 
    // (to . from)/LenSq(to) * to
    T to_len_sq = Vec_LengthSq(to);
    
    T to_dot_from = Vec_Dot(to, from);
    T unit_projection_scalar = to_dot_from / to_len_sq;
    Vec<T,N> ret = to * unit_projection_scalar;
    return ret;
}


template<typename T, u32 N>
static inline Vec<T,N>
Vec_Cross(Vec<T,N> lhs, Vec<T,N> rhs) {
    // NOTE(Momo): Cross product only exist in 3 and 7
    // For now, we only care about 3
    static_assert(N == 3);
    return {
        (lhs[1] * rhs[2]) - (lhs[2] * rhs[1]),
        (lhs[2] * rhs[0]) - (lhs[0] * rhs[2]),
        (lhs[0] * rhs[1]) - (lhs[1] * rhs[0])
    };
}

template<typename T>
struct Vec<T,2> {
    union {
        T e[2];
        struct { T x, y; };
        struct { T u, v; };
        struct { T w, h; };
    };
    
    inline auto& operator[](u32 i) { 
        Assert(i < 2); 
        return e[i]; 
    }
    
};


template<typename T>
struct Vec<T,3> {
    union {
        T e[3];
        struct {
            union {
                Vec<T,2> xy;
                struct { T x, y; };
            };
            T z;
        };
        
        struct {
            union {
                Vec<T,2> wh;
                struct {
                    T w, h;
                };
            };
            T d;
        }; 
    };
    inline auto& operator[](u32 i) { 
        Assert(i < 3); 
        return e[i]; 
    }
};

template<typename T>
struct Vec<T,4> {
    union {
        T e[4];
        struct {
            union {
                Vec<T,3> xyz;
                struct { T x, y, z; };
                struct {
                    union { Vec<T,2> xy; };
                    T z;
                };
            }; 
            T w;
        };
    };
    
    
    inline auto& operator[](u32 i) { 
        Assert(i < 4); 
        return e[i]; 
    }
    
};

typedef Vec<f32,2> Vec2f;
typedef Vec<u32,2> Vec2u;
typedef Vec<s32,2> Vec2s;
typedef Vec<f32,3> Vec3f;
typedef Vec<u32,3> Vec3u;
typedef Vec<s32,3> Vec3s;
typedef Vec<f32,4> Vec4f;
typedef Vec<u32,4> Vec4u;
typedef Vec<s32,4> Vec4s;


static inline Vec2f 
Vec_Rotate(Vec2f v, f32 rad) {
    // Technically, we can use matrices but
    // meh, it's easy to code this out without it.
    // Removes dependencies too
    f32 c = Cos(rad);
    f32 s = Sin(rad);
    
    Vec2f ret = {};
    ret.x = (c * v.x) - (s * v.y);
    ret.y = (s * v.x) + (c * v.y);
    return ret;
}

template<typename T, typename... Args>
static inline Vec<T, sizeof...(Args)+(1)>
Vec_Create(T x, Args... args) {
    return { x, args... };
}

#if 0
//~ NOTE(Momo): Vectors
struct v2f {
    union {
        f32 elements[2];
        struct {
            f32 x;
            f32 y;
        };
        struct {
            f32 u;
            f32 v;
        };
        struct {
            f32 w;
            f32 h;
        };
    };
    
    inline f32& operator[](u32 i);
    
};

struct v2u {
    union {
        u32 elements[2];
        struct {
            u32 x;
            u32 y;
        };
        struct {
            u32 w;
            u32 h;
        };
    };
    
    inline u32& operator[](u32 i);
    
};

struct v2s {
    union {
        s32 elements[2];
        struct {
            s32 x;
            s32 y;
        };
        struct {
            s32 w;
            s32 h;
        };
    };
    inline auto& operator[](u32 i) { 
        Assert(i < 2); 
        return elements[i]; 
    }
};

struct v3f {
    union {
        f32 elements[3];
        struct {
            union {
                v2f xy;
                struct {
                    f32 x, y;
                };
            };
            f32 z;
        };
        
        struct {
            union {
                v2f wh;
                struct {
                    f32 w, h;
                };
            };
            f32 d;
        }; 
    };
    
    inline f32& operator[](u32 i);
};

struct v3s {
    union {
        s32 elements[3];
        struct {
            union {
                v2s xy;
                struct {
                    s32 x, y;
                };
            };
            s32 z;
        };
        
        struct {
            union {
                v2s wh;
                struct {
                    s32 w, h;
                };
            };
            s32 d;
        }; 
    };
    
    inline s32& operator[](u32 i);
};

struct v4f {
    union {
        f32 elements[4];
        struct {
            union {
                v3f xyz;
                struct {
                    f32 x, y, z;
                };
                struct {
                    union {
                        v2f xy;
                    };
                    f32 z;
                };
            }; 
            f32 w;
        };
        
        
    };
    
    inline f32& operator[](u32 i);
    
};


//~ NOTE(Momo): v2f functions
f32&
v2f::operator[](u32 i) {
    Assert(i < 2); 
    return elements[i]; 
}

v2f 
v2f_Create(f32 x, f32 y)  {
    return { x, y };
}

static inline v2f 
v2f_Add(v2f l, v2f r) {
    l.x += r.x;
    l.y += r.y;
    return l;
}

static inline v2f 
operator+(v2f lhs, v2f rhs) {
    return v2f_Add(lhs, rhs);
}

static inline v2f&
operator+=(v2f& lhs, v2f rhs) {
    return lhs = lhs + rhs;
}


static inline v2f
v2f_Sub(v2f l, v2f r) {
    l.x -= r.x;
    l.y -= r.y;
    return l;
}


static inline v2f 
operator-(v2f lhs, v2f rhs) {
    return v2f_Sub(lhs, rhs);
}

static inline v2f&
operator-=(v2f& lhs, v2f rhs) {
    return lhs = lhs - rhs;
}

static inline v2f
v2f_Mul(v2f l, f32 r) {
    l.x *= r;
    l.y *= r;
    return l;
}


static inline v2f 
operator*(v2f lhs, f32 rhs) {
    return v2f_Mul(lhs, rhs);
}

static inline v2f 
operator*=(v2f lhs, f32 rhs) {
    return lhs = lhs * rhs;
}

static inline v2f 
operator*(f32 lhs, v2f rhs) {
    return v2f_Mul(rhs, lhs);
}


static inline v2f
v2f_Div(v2f l, f32 r) {
    l.x /= r;
    l.y /= r;
    
    return l;
}

static inline v2f
v2f_Ratio(v2f l, v2f r) {
    l.x /= r.x;
    l.y /= r.y;
    
    return l;
}

static inline v2f
v2f_Negate(v2f v){
    v.x = -v.x;
    v.y = -v.y;
    return v;
}

static inline b8 
v2f_IsEqual(v2f l, v2f r) {
    return 
        IsEqual(l.x, r.x) && 
        IsEqual(l.y, r.y);
}

static inline b8
operator==(v2f l, v2f r) {
    return v2f_IsEqual(l,r);
}

static inline b8
operator!=(v2f l, v2f r) {
    return !(l == r);
}

static inline f32 
v2f_Dot(v2f l, v2f r) {
    f32 ret = {};
    ret = l.x * r.x + l.y * r.y;
    return ret;
}

static inline v2f 
v2f_Midpoint(v2f l, v2f r)  {
    return (l + r) * 0.5f; 
}

static inline f32
v2f_LengthSq(v2f v) { 
    // NOTE(Momo): Dot Product trick!
    return v2f_Dot(v, v);
}


static inline f32 
v2f_Length(v2f l)  { 
    return Sqrt(v2f_LengthSq(l));
}

static inline f32
v2f_DistanceSq(v2f l, v2f r) {
    return v2f_LengthSq(r - l);
}

static inline f32
v2f_Distance(v2f l, v2f r)  { 
    return Sqrt(v2f_DistanceSq(l, r)); 
}

static inline v2f 
v2f_Normalize(v2f V)  {
    f32 len = v2f_Length(V);
    v2f ret = v2f_Div(V, len);
    return ret;
}

static inline f32
v2f_AngleBetween(v2f l, v2f r) {
    f32 l_len = v2f_Length(l);
    f32 r_len = v2f_Length(r);
    f32 lr_dot = v2f_Dot(l,r);
    f32 ret = Acos(lr_dot/(l_len * r_len));
    return ret;
}

static inline b8
v2f_IsPerpendicular(v2f l, v2f r) { 
    f32 lr_dot = v2f_Dot(l,r);
    return IsEqual(lr_dot, 0.f); 
}


static inline b8 
v2f_IsSameDirection(v2f l, v2f r) { 
    f32 lr_dot = v2f_Dot(l,r);
    return lr_dot > 0; 
}


static inline b8 
v2f_IsOppositeDirection(v2f l, v2f r) { 
    f32 lr_dot = v2f_Dot(l,r);
    return lr_dot < 0;
}

static inline v2f 
v2f_Project(v2f from, v2f to) { 
    // (to . from)/LenSq(to) * to
    f32 to_len_sq = v2f_LengthSq(to);
    
    f32 to_dot_from = v2f_Dot(to, from);
    f32 unit_projection_scalar = to_dot_from / to_len_sq;
    v2f ret = to * unit_projection_scalar;
    return ret;
}

static inline v2f
Rotate(v2f v, f32 rad) {
    // Technically, we can use matrices but
    // meh, it's easy to code this out without it.
    // Removes dependencies too
    f32 c = Cos(rad);
    f32 s = Sin(rad);
    
    v2f ret = {};
    ret.x = (c * v.x) - (s * v.y);
    ret.y = (s * v.x) + (c * v.y);
    return ret;
}

//~ NOTE(Momo): v2u Functions
u32&
v2u::operator[](u32 i) {
    Assert(i < 2); 
    return elements[i]; 
}

v2u 
v2u_Create(u32 x = 0.f, u32 y = 0.f)  {
    return { x, y };
}

//~ NOTE(Momo): v2s Functions
static inline v2s 
Add(v2s l, v2s r) {
    l.x += r.x;
    l.y += r.y;
    return l;
}

static inline v2s
operator+(v2s l, v2s r) {
    return Add(l,r);
}

static inline v2s
Sub(v2s l, v2s r) {
    l.x -= r.x;
    l.y -= r.y;
    return l;
}


static inline v2s
operator-(v2s l, v2s r) {
    return Sub(l,r);
}

static inline v2s
Mul(v2s l, f32 r) {
    l.x = s32(l.x * r);
    l.y = s32(l.y * r);
    return l;
}


static inline v2s 
operator*(v2s lhs, f32 rhs) {
    return Mul(lhs, rhs);
}

//~ NOTE(Momo): v3s Functions
s32&
v3s::operator[](u32 i) {
    Assert(i < 3); 
    return elements[i]; 
}

v3s 
v3s_Create(s32 x = 0, s32 y = 0, s32 z = 0)  {
    return { x, y, z };
}

v3s
Cross(v3s lhs, v3s rhs) {
    return {
        (lhs.y * rhs.z) - (lhs.z * rhs.y),
        (lhs.z * rhs.x) - (lhs.x * rhs.z),
        (lhs.x * rhs.y) - (lhs.y * rhs.x)
    };
}


//~ NOTE(Momo): v3f Functions
f32&
v3f::operator[](u32 i) {
    Assert(i < 3); 
    return elements[i]; 
}

v3f 
v3f_Create(f32 x = 0, f32 y = 0, f32 z = 0)  {
    return { x, y, z };
}

static inline v3f 
Add(v3f l, v3f r) {
    l.x += r.x;
    l.y += r.y;
    l.z += r.z;
    return l;
}

static inline v3f
operator+(v3f l, v3f r) {
    return Add(l,r);
}

static inline v3f&
operator+=(v3f& l, v3f r) {
    return l = l + r;
}


static inline v3f
Sub(v3f l, v3f r) {
    l.x -= r.x;
    l.y -= r.y;
    l.z -= r.z;
    return l;
}


static inline v3f
operator-(v3f l, v3f r) {
    return Sub(l,r);
}

static inline v3f&
operator-=(v3f& l, v3f r) {
    return l = l - r;
}



static inline v3f
Mul(v3f l, f32 r) {
    l.x *= r;
    l.y *= r;
    l.z *= r;
    return l;
}


static inline v3f 
operator*(v3f lhs, f32 rhs) {
    return Mul(lhs, rhs);
}

static inline v3f 
operator*=(v3f lhs, f32 rhs) {
    return lhs = lhs * rhs;
}

static inline v3f 
operator*(f32 lhs, v3f rhs) {
    return Mul(rhs, lhs);
}


static inline v3f
Div(v3f l, f32 r) {
    Assert(!IsEqual(r, 0));
    l.x /= r;
    l.y /= r;
    l.z /= r;
    return l;
}

static inline v3f
neg(v3f v){
    v.x = -v.x;
    v.y = -v.y;
    v.z = -v.z;
    return v;
}

static inline b8 
IsEqual(v3f l, v3f r) {
    return 
        IsEqual(l.x, r.x) && 
        IsEqual(l.y, r.y) &&
        IsEqual(l.z, r.z);
}

static inline f32 
Dot(v3f l, v3f r) {
    f32 ret = {};
    ret = l.x * r.x + l.y * r.y + l.z * r.z;
    return ret;
}

static inline v3f 
Midpoint(v3f l, v3f r)  { 
    return (l + r) * 0.5f; 
}

static inline f32
LengthSq(v3f V) { 
    // NOTE(Momo): Dot Product trick!
    return Dot(V, V);
}

static inline f32
DistanceSq(v3f l, v3f r) {
    f32 ret = LengthSq(l - r); 
    return ret;
}

static inline f32
Distance(v3f l, v3f r)  { 
    return Sqrt(DistanceSq(l, r)); 
}

static inline f32 
Length(v3f l)  { 
    return Sqrt(LengthSq(l));
}

static inline v3f 
Normalize(v3f v)  {
    f32 len = Length(v);
    v3f ret = Div(v, len);
    return ret;
}

static inline f32
AngleBetween(v3f L, v3f R) {
    f32 l_len = Length(L);
    f32 r_len = Length(R);
    f32 LRDot = Dot(L,R);
    f32 Ret = Acos(LRDot/(l_len * r_len));
    
    return Ret;
}


static inline b8
IsPerpendicular(v3f l, v3f r) { 
    f32 lr_dot = Dot(l,r);
    return IsEqual(lr_dot, 0); 
}


static inline b8 
IsSameDirection(v3f l, v3f r) { 
    f32 lr_dot = Dot(l,r);
    return lr_dot > 0; 
}


static inline b8 
IsOppositeDirection(v3f l, v3f r) { 
    f32 lr_dot = Dot(l,r);
    return lr_dot < 0;
}

static inline v3f 
Project(v3f from, v3f to) { 
    // (to . from)/LenSq(to) * to
    f32 to_len_sq = LengthSq(to);
    
    f32 to_dot_from = Dot(to, from);
    f32 unit_projection_scalar = to_dot_from / to_len_sq;
    v3f ret = Mul(to, unit_projection_scalar);
    return ret;
}


//~ NOTE(Momo): v4f 
f32&
v4f::operator[](u32 i) {
    Assert(i < 4); 
    return elements[i]; 
}

v4f
v4f_Create(f32 x = 0, f32 y = 0, f32 z = 0, f32 w = 0)  {
    return { x, y, z, w };
}


static inline v3f 
v3f_Create(v2f v) {
    v3f ret = {};
    ret.x = v.x;
    ret.y = v.y;
    ret.z = 0.f;
    
    return ret;
}

static inline v2f 
v2f_Create(v2s v) {
    return { f32(v.x), f32(v.y) };
}

static inline v2f
v2f_Create(v2u v) {
    return { f32(v.x), f32(v.y) };
}

static inline v2s 
v2s_Create(v2f v) {
    return { s32(v.x), s32(v.y) };
}
#endif

#endif //MOMO_VECTOR_H
