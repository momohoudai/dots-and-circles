#ifndef __MOMO_C4f__
#define __MOMO_C4f__

// TODO(Momo): revamp
struct c3f {
    union {
        f32 e[3];
        struct {
            f32 r, g, b;
        };
    };
};

struct c4f {
    union {
        f32 e[4];
        struct {
            union {
                Vec3f rgb;
                struct {
                    f32 r, g, b;
                };
            }; 
            f32 a;
        };
    };
    
};

static inline c4f
c4f_Create(f32 r, f32 g, f32 b, f32 a){
    return { r, g, b, a };
}



#define C4F_GREY1  c4f_Create(0.1f, 0.1f, 0.1f, 1.f)
#define C4F_GREY2  c4f_Create(0.2f, 0.2f, 0.2f, 1.f)
#define C4F_GREY3  c4f_Create(0.3f, 0.3f, 0.3f, 1.f)
#define C4F_GREY4  c4f_Create(0.4f, 0.4f, 0.4f, 1.f)
#define C4F_GREY5  c4f_Create(0.5f, 0.5f, 0.5f, 1.f)
#define C4F_GREY6  c4f_Create(0.6f, 0.6f, 0.6f, 1.f)
#define C4F_GREY7  c4f_Create(0.7f, 0.7f, 0.7f, 1.f)
#define C4F_GREY8  c4f_Create(0.8f, 0.8f, 0.8f, 1.f)
#define C4F_GREY9  c4f_Create(0.9f, 0.9f, 0.9f, 1.f)
#define C4F_WHITE  c4f_Create(1.f, 1.f, 1.f, 1.f)
#define C4F_BLACK  c4f_Create(0.f, 0.f, 0.f, 1.f)
#define C4F_RED  c4f_Create(1.f, 0.f, 0.f, 1.f)
#define C4F_GREEN  c4f_Create(0.f, 1.f, 0.f, 1.f)
#define C4F_BLUE  c4f_Create(0.f, 0.f, 1.f, 1.f)
#define C4F_YELLOW  c4f_Create(1.f, 1.f, 0.f, 1.f)

#endif 
