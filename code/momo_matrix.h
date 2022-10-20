/* date = June 27th 2021 3:54 pm */

#ifndef MOMO_MATRIX_H
#define MOMO_MATRIX_H

// NOTE(Momo): For now, we only cater for square matrices
template<typename T, u32 N>
struct Mat {
    Vec<T,N> e[N];
    inline Vec<T,N>& operator[](u32 index){
        Assert(index < N); 
        return e[index]; 
    }
};

typedef Mat<f32,2> Mat2f;
typedef Mat<f32,4> Mat4f;

template<typename T, u32 N>
static inline Mat<T,N>
Mat_Concat(Mat<T,N> lhs, Mat<T,N> rhs) {
    Mat<T,N> ret = {};
    for (u8 r = 0; r < N; r++) { 
        for (u8 c = 0; c < N; c++) { 
            for (u8 i = 0; i < N; i++) 
                ret[r][c] += lhs[r][i] *  rhs[i][c]; 
        } 
    } 
    return ret;
}

template<typename T, u32 N>
static inline Vec<T,N>
Mat_Concat(Mat<T,N> l, Vec<T,N> r) {
    Vec<T,N> ret = {};
    for (u32 i = 0; i < N; ++i) {
        for (u32 j = 0; j < N; ++j) {
            ret[i] += l[i][j] * r[j];
        }
    }
    
    return ret;
}

template<typename T, u32 N>
static inline Vec<T,N> 
operator*(Mat<T,N> l, Vec<T,N> r) {
    return Mat_Concat(l,r);
}



template<typename T, u32 N>
static inline Mat<T,N>
operator*(Mat<T,N> lhs, Mat<T,N> rhs) {
    return Mat_Concat(lhs, rhs);
}

template<typename T, u32 N>
static inline Mat<T,N> 
Mat_Identity() {
    Mat<T,N> ret = {};
    for (u32 i = 0 ; i < N; ++i) {
        ret[i][i] = T(1);
    }
    return ret;
}

template<typename T, u32 N>
static inline Mat<T,N> 
Mat_Transpose(Mat<T,N> m) {
    Mat<T,N> ret = {};
    for (u32 i = 0; i < N; ++i ) {
        for (u32 j = 0; j < N; ++j) {
            ret[i][j] = m[j][i];
        }
    }
    
    return ret;
}


template<typename T, u32 N>
static inline Mat<T,N>
Mat_Translation(Vec<T,N> v) {
    // NOTE(Momo): 
    // 1 0 0 x
    // 0 1 0 y
    // 0 0 1 z
    // 0 0 0 1
    //
    Mat4f ret = Mat_Identity<T,4>();
    for (u32 i = 0; i < N-1; ++i){
        ret[i][N-1] = v[i];
    }
    return ret;
}

template<typename T>
static inline Mat<T,4>
Mat_Translation(T x, T y, T z) {
    return Mat_Translation(Vec<T,4>{x, y, z});
}

template<typename T, u32 N>
static inline Mat<T,N>
Mat_Scale(Vec<T,N> v) {
    // NOTE(Momo): 
    //  x  0  0  0
    //  0  y  0  0
    //  0  0  z  0
    //  0  0  0  1
    Mat<T,N> ret = {};
    for (u32 i = 0; i < N-1; ++i) {
        ret[i][i] = v[i];
    }
    ret[N-1][N-1] = 1;
    
    return ret; 
}

template<typename T>
static inline Mat<T,4>
Mat_Scale(T x, T y, T z) {
    return Mat_Scale(Vec<T,4>{x, y, z});
}

//~ Non-generic ones
static inline Mat4f 
Mat4f_RotationX(f32 rad) {
    // NOTE(Momo): 
    // 1  0  0  0
    // 0  c -s  0
    // 0  s  c  0
    // 0  0  0  1
    
    f32 c = Cos(rad);
    f32 s = Sin(rad);
    Mat4f ret = {};
    ret[0][0] = 1.f;
    ret[3][3] = 1.f;
    ret[1][1] = c;
    ret[1][2] = -s;
    ret[2][1] = s;
    ret[2][2] = c;
    
    return ret;
}

static inline Mat4f 
Mat4f_RotationY(f32 rad) {
    // NOTE(Momo): 
    //  c  0  s  0
    //  0  1  0  0
    // -s  0  c  0
    //  0  0  0  1
    
    f32 c = Cos(rad);
    f32 s = Sin(rad);
    Mat4f ret = {};
    ret[0][0] = c;
    ret[0][2] = s;
    ret[1][1] = 1.f;
    ret[2][0] = -s;
    ret[2][2] = c;
    ret[3][3] = 1.f;
    
    return ret;
}

static inline Mat4f 
Mat4f_RotationZ(f32 rad) {
    // NOTE(Momo): 
    //  c -s  0  0
    //  s  c  0  0
    //  0  0  1  0
    //  0  0  0  1
    
    f32 c = Cos(rad);
    f32 s = Sin(rad);
    Mat4f ret = {};
    ret[0][0] = c;
    ret[0][1] = -s;
    ret[1][0] = s;
    ret[1][1] = c;
    ret[2][2] = 1.f;
    ret[3][3] = 1.f;
    
    return ret;
}
// NOTE(Momo): Assumes that NDC is within [-1, 1] 
// and right hand rule
static inline Mat4f
Mat4f_Orthographic(f32 left, f32 right, 
                   f32 bottom, f32 top,
                   f32 near, f32 far) 
{
    Mat4f ret = {};
    ret[0][0] = 2.f/(right-left);
    ret[1][1] = 2.f/(top-bottom);
    ret[2][2] = 2.f/(far-near);
    ret[3][3] = 1.f;
    ret[0][3] = -(right+left)/(right-left);
    ret[1][3] = -(top+bottom)/(top-bottom);
    ret[2][3] = -(far+near)/(far-near);
    
    return ret;
}

// NOTE(Momo): Assumes that NDC is within [-1, 1] 
// and right hand rule
static inline Mat4f 
Mat4f_Frustum(f32 left, f32 right, 
              f32 bottom, f32 top,
              f32 near, f32 far) 
{
    Mat4f ret = {};
    ret[0][0] = (2.f*near)/(right-left);
    ret[1][1] = (2.f*near)/(top-bottom);
    ret[2][2] = -(far+near)/(far-near);
    ret[3][2] = 1;  
    ret[0][2] = (right+left)/(right-left);
    ret[1][2] = (top+bottom)/(top-bottom);
    ret[1][3] = -near*(top+bottom)/(top-bottom);
    ret[2][3] = 2.f*far*near/(far-near);
    
    return ret;
}

// NOTE(Momo): Assumes that NDC is within [-1, 1] 
// and right hand rule
static inline Mat4f 
Mat4f_Perspective(f32 fov, f32 aspect,
                  f32 near, f32 far) 
{
    f32 top = near * Tan(fov*0.5f);
    f32 right = top * aspect;
    return Mat4f_Frustum(-right, right,
                         -top, top,
                         near, far);
}


#endif //MOMO_MATRIX_H
