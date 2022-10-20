/* NOTE(Momo): 
Ground rules about this renderer.
- This is a 2D renderer in 3D space. 
- Right-handed coordinate system: +Y is up, +Z is towards you
- Only one model is supported: A quad that can be textured and colored 
- UV origin is from top left. Points given for UV to map to the quad is given in this order:
>> 0. Top left
>> 1. Top right
>> 2. Bottom right
>> 3. Bottom left

- Indices layout 2 triangles in the following fashion:
* ---
 * |/|
 * ---
*/
#ifndef RENDERER_H
#define RENDERER_H

// NOTE(Momo): Header for all renderer objects
struct Renderer {	
	Mailbox render_commands;
};

struct Renderer_Texture_Handle {
    b8 success;
    u32 id;
};


enum Renderer_Command_Type {
    Renderer_CommandType_ClearColor,
    Renderer_CommandType_SetBasis,
    Renderer_CommandType_DrawTexturedQuad,
    Renderer_CommandType_DrawQuad,
};

struct Renderer_Command_ClearColor {
    c4f colors;
};

struct Renderer_Command_SetBasis {
    Mat4f basis;
};

struct Renderer_Command_DrawTexturedQuad {
    Renderer_Texture_Handle texture_handle;
    c4f colors;
    Mat4f transform;
    Rect2f texture_uv_rect; 
};


struct Renderer_Command_DrawQuad {
    c4f colors;
    Mat4f transform;
};

struct Renderer_Command_SetDesignResolution {
    u32 width;
    u32 height;
};

static inline Rect2f
Renderer_CreateDefaultUV() {
    Rect2f ret = {};
    ret.max = { 1.f, 1.f };
    return ret;
}




static inline void
Renderer_SetBasis(Mailbox* commands, Mat4f basis) {
    auto* data = Mailbox_Push<Renderer_Command_SetBasis>(commands, Renderer_CommandType_SetBasis);
    data->basis = basis;
}

static inline void
Renderer_SetOrthoCamera(Mailbox* commands, 
                        Vec3f position,
                        Rect3f frustum)   
{
    auto* data = Mailbox_Push<Renderer_Command_SetBasis>(commands, Renderer_CommandType_SetBasis);
    
    auto p  = Mat4f_Orthographic(frustum.min.x,  
                                 frustum.max.x, 
                                 frustum.min.y, 
                                 frustum.max.y,
                                 frustum.min.z, 
                                 frustum.max.z);
    
    Mat4f v = Mat_Translation(-position.x, -position.y, -position.z);
    data->basis = p * v;
}

static inline void
Renderer_ClearColor(Mailbox* commands, c4f colors) {
    auto* data = Mailbox_Push<Renderer_Command_ClearColor>(commands, Renderer_CommandType_ClearColor);
    data->colors = colors;
}

static inline void
Renderer_DrawTexturedQuad(Mailbox* commands, 
                          c4f colors, 
                          Mat4f transform, 
                          Renderer_Texture_Handle texture_handle,
                          Rect2f texture_uv_rect = Renderer_CreateDefaultUV())  

{
    auto* data = Mailbox_Push<Renderer_Command_DrawTexturedQuad>(commands, Renderer_CommandType_DrawTexturedQuad);
    
    data->colors = colors;
    data->transform = transform;
    data->texture_handle = texture_handle;
    data->texture_uv_rect = texture_uv_rect;
}

static inline void
Renderer_DrawQuad(Mailbox* commands, 
                  c4f colors, 
                  Mat4f transform) 
{
    auto* data = Mailbox_Push<Renderer_Command_DrawQuad>(commands, Renderer_CommandType_DrawQuad);
    data->colors = colors;
    data->transform = transform;
}

static inline void 
Renderer_DrawLine(Mailbox* commands, 
                  Line2f line,
                  f32 thickness,
                  c4f colors,
                  f32 pos_z) 
{
    // NOTE(Momo): Min.Y needs to be lower than Max.y
    if (line.min.y > line.max.y) {
        Swap(line.min, line.max);
    }
    
    Vec2f line_vector = line.max - line.min;
    f32 line_length = Vec_Length(line_vector);
    Vec2f line_mid = Vec_Midpoint(line.max, line.min);
    
    Vec2f x_axis = { 1.f, 0.f };
    f32 angle = Vec_AngleBetween(line_vector, x_axis);
    
    Mat4f T = Mat_Translation(line_mid.x, line_mid.y, pos_z);
    Mat4f R = Mat4f_RotationZ(angle);
    Mat4f S = Mat_Scale(line_length, thickness, 1.f) ;
    
    Renderer_DrawQuad(commands, colors, T*R*S);
}

static inline void
Renderer_DrawCircle(Mailbox* commands,
                    Circ2f circle,
                    f32 thickness, 
                    u32 line_count,
                    c4f color,
                    f32 pos_z) 
{
    // NOTE(Momo): Essentially a bunch of lines
    // We can't really have a surface with less than 3 lines
    Assert(line_count >= 3);
    f32 angle_increment = TAU / line_count;
    Vec2f pt1 = { 0.f, circle.radius }; 
    Vec2f pt2 = Vec_Rotate(pt1, angle_increment);
    
    for (u32 I = 0; I < line_count; ++I) {
        Vec2f line_pt_1 = pt1 + circle.origin;
        Vec2f line_pt_2 = pt2 + circle.origin;
        Line2f line = { line_pt_1, line_pt_2 };
        Renderer_DrawLine(commands, 
                          line,
                          thickness,
                          color,
                          pos_z);
        
        pt1 = pt2;
        pt2 = Vec_Rotate(pt1, angle_increment);
        
    }
}

static inline void 
Renderer_DrawAabb(Mailbox* commands, 
                  Rect2f rect,
                  f32 thickness,
                  c4f colors,
                  f32 pos_z ) 
{
    //Bottom
    Renderer_DrawLine(commands, 
                      Line2f{
                          rect.min.x, 
                          rect.min.y,  
                          rect.max.x, 
                          rect.min.y
                      },
                      thickness, 
                      colors,
                      pos_z);
    // Left
    Renderer_DrawLine(commands, 
                      Line2f{
                          rect.min.x,
                          rect.min.y,
                          rect.min.x,
                          rect.max.y
                      },  
                      thickness, 
                      colors,
                      pos_z);
    
    //Top
    Renderer_DrawLine(commands, 
                      Line2f{
                          rect.min.x,
                          rect.max.y,
                          rect.max.x,
                          rect.max.y
                      }, 
                      thickness, 
                      colors,
                      pos_z);
    
    //Right 
    Renderer_DrawLine(commands, 
                      Line2f{
                          rect.max.x,
                          rect.min.y,
                          rect.max.x,
                          rect.max.y
                      },  
                      thickness, 
                      colors,
                      pos_z);
}

#endif //GAME_RENDERER_H
