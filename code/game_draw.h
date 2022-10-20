#ifndef __GAME_DRAW__
#define __GAME_DRAW__

// NOTE(Momo): These are functions combining game_renderer.h and game_assets.h.
// Mostly shortcut functions to draw items
struct Painter {
    Mailbox* render_commands;
    Assets* assets;
    f32 current_layer;
};

static inline void
Painter__GoToNextLayer(Painter* p) {
    p->current_layer -= 0.01f;
}

static inline void
Painter_Prepare(Painter* p,
                Mailbox* render_commands,
                Assets* assets,
                c4f color) 
{
    p->render_commands = render_commands;
    p->assets = assets;
    p->current_layer = GAME_DESIGN_DEPTH - 0.1f;
    
    Rect3f center_box = Rect_CreateDims(Vec_Create((f32)GAME_DESIGN_WIDTH, 
                                                   (f32)GAME_DESIGN_HEIGHT, 
                                                   (f32)GAME_DESIGN_DEPTH));
    Renderer_ClearColor(p->render_commands, color);
    Renderer_SetOrthoCamera(p->render_commands,
                            {}, 
                            center_box);
}

static inline void
Painter_DrawCircle(Painter* p,
                   Circ2f circle,
                   f32 thickness,
                   u32 partitions,
                   c4f color) {
    Renderer_DrawCircle(p->render_commands, 
                        circle,
                        thickness,
                        partitions,
                        color,
                        p->current_layer);
    
    Painter__GoToNextLayer(p);
}


static inline void
Painter_DrawQuad(Painter* p,
                 Vec2f position,
                 Vec2f scale,
                 c4f color = C4F_WHITE) 
{
    Mat4f transform = {
        scale.x, 0.f, 0.f, position.x,
        0.f, scale.y, 0.f, position.y,
        0.f, 0.f, 1.f, p->current_layer,
        0.f, 0.f, 0.f, 1.f
    };
    
    Painter__GoToNextLayer(p);
    Renderer_DrawQuad(p->render_commands,
                      color,
                      transform);
    
}

static inline void 
Painter_DrawLine(Painter* p, 
                 Line2f line,
                 f32 thickness,
                 c4f colors) 
{
    Painter__GoToNextLayer(p);
    Renderer_DrawLine(p->render_commands, line, thickness, colors, p->current_layer);
    
}


static inline void
Painter_DrawImage(Painter* p,
                  Image_ID image_id,
                  Vec2f position,
                  Vec2f scale,
                  c4f color = C4F_WHITE) 
{
    Image* image = Assets_GetImage(p->assets, image_id);
    if (!Rect_IsValid(image->texture_uv_rect)){
        return;
    }
    Texture* texture = Assets_GetTexture(p->assets, image->texture_id);
    
    Mat4f transform = {
        scale.x, 0.f, 0.f, position.x,
        0.f, scale.y, 0.f, position.y,
        0.f, 0.f, 1.f, p->current_layer,
        0.f, 0.f, 0.f, 1.f
    };
    
    Painter__GoToNextLayer(p);
    
    Renderer_DrawTexturedQuad(p->render_commands,
                              color,
                              transform,
                              texture->handle,
                              image->texture_uv_rect);
}

static inline void
Painter_DrawImage(Painter* p,
                  Image_ID image_id,
                  Vec2f position,
                  Vec2f scale,
                  f32 rotation,
                  c4f color = C4F_WHITE) 
{
    Image* image = Assets_GetImage(p->assets, image_id);
    if (!Rect_IsValid(image->texture_uv_rect)){
        return;
    }
    Texture* texture = Assets_GetTexture(p->assets, image->texture_id);
    
    Mat4f s = Mat_Scale(scale.x, scale.y, 1.f);
    Mat4f r = Mat4f_RotationZ(rotation);
    Mat4f t = Mat_Translation(position.x,
                              position.y,
                              p->current_layer);
    Mat4f transform = t*r*s;
    
    
    Painter__GoToNextLayer(p);
    Renderer_DrawTexturedQuad(p->render_commands,
                              color,
                              transform,
                              texture->handle,
                              image->texture_uv_rect);
}


// NOTE(Momo): Left aligned
static inline void
Painter_DrawTextLeft(Painter* p,
                     Font_ID font_id,
                     Vec2f position,
                     String str,
                     f32 size, 
                     c4f color = C4F_WHITE) 
{
    Vec2f cur_position = position;
    Font* font = Assets_GetFont(p->assets, font_id);
    Texture* texture = Assets_GetTexture(p->assets, font->texture_id);
    
    for(u32 i = 0; i < str.count; ++i) {
        Font_Glyph* glyph = Font_GetGlyph(font, str.data[i]);
        if (Rect_IsValid(glyph->texture_uv_rect)) {
            
            Rect2f box = glyph->glyph_box; 
            
            f32 box_w = Rect_Width(box);
            f32 box_h = Rect_Height(box);
            
            
            // NOTE(Momo): Set bottom left as origin
            Mat4f a = Mat_Translation(0.5f, 0.5f, 0.f); 
            
            
            Mat4f s = Mat_Scale(box_w * size, 
                                box_h* size, 
                                1.f);
            
            Mat4f t = Mat_Translation(cur_position.x + box.min.x * size, 
                                      cur_position.y + box.min.y * size,  
                                      p->current_layer);
            
            Painter__GoToNextLayer(p);
            
            Renderer_DrawTexturedQuad(p->render_commands,
                                      color,
                                      t*s*a,
                                      texture->handle,
                                      glyph->texture_uv_rect);
        }
        
        cur_position.x += glyph->advance * size;
        //g_log("%f\n", glyph->advance * size);
        if (i != str.count - 1 ) {
            u32 kerning = Font_GetKerning(font, str.data[i], str.data[i+1]);
            cur_position.x += kerning * size;
        }
        
    }
}

static inline void
Painter_DrawTextLeft(Painter* p,
                     Font_ID font_id,
                     Vec2f position,
                     Msg_ID msg_id,
                     f32 size, 
                     c4f color = C4F_WHITE) 
{
	String str = *(Assets_GetMsg(p->assets, msg_id));
	Painter_DrawTextLeft(p, font_id, position, str, size, color);
}
// NOTE(Momo): Center aligned
// A bit inefficient because we have to precalculate
// the whole string first, to get the width of the string,
// then we have to offset by half and THEN draw the string
// as if it's left justified
static inline void
Painter_DrawTextCenter(Painter* p,
                       Font_ID font_id,
                       Vec2f position,
                       String str,
                       f32 size, 
                       c4f color = C4F_WHITE) 
{
    Vec2f cur_position = position;
    Vec2f left_aligned_pos = position;
    Font* font = Assets_GetFont(p->assets, font_id);
    for(u32 i = 0; i < str.count; ++i) {
        Font_Glyph* glyph = Font_GetGlyph(font, str.data[i]);
        
        cur_position.x += glyph->advance * size;
        if (i != str.count - 1 ) {
            u32 kerning = Font_GetKerning(font, str.data[i], str.data[i+1]);
            cur_position.x += kerning * size;
        }
    }
    f32 string_w = cur_position.x - position.x;
    left_aligned_pos.x = position.x - (string_w * 0.5f);
    
    Painter_DrawTextLeft(p, font_id, left_aligned_pos, str, size, color);
    
}
static inline void
Painter_DrawTextCenter(Painter* p,
					   Font_ID font_id,
					   Vec2f position,
					   Msg_ID msg_id,
					   f32 size, 
					   c4f color = C4F_WHITE) 
{
	String str = *(Assets_GetMsg(p->assets, msg_id));
	Painter_DrawTextCenter(p, font_id, position, str, size, color);
}
static inline void
Painter_DrawTextRight(Painter* p, 
                      Font_ID font_id,
                      Vec2f position,
                      String str,
                      f32 size, 
                      c4f color = C4F_WHITE) 
{
    Vec2f cur_position = position;
    Font* font = Assets_GetFont(p->assets, font_id);
    Texture* texture = Assets_GetTexture(p->assets, font->texture_id);
    
    for(u32 i = 0; i < str.count; ++i) {
        u32 index = str.count - i - 1;
        Font_Glyph* glyph = Font_GetGlyph(font, str.data[index]);
        if (Rect_IsValid(glyph->texture_uv_rect)) {
            Rect2f box = glyph->glyph_box; 
            
            // NOTE(Momo): Set bottom right as origin
            Mat4f A = Mat_Translation(-0.5f, 0.5f, 0.f); 
            
            f32 box_w = Rect_Width(box);
            f32 box_h = Rect_Height(box);
            
            Mat4f S = Mat_Scale(box_w* size, 
                                box_h* size, 
                                1.f);
            
            
            Mat4f T = Mat_Translation(cur_position.x - (glyph->advance - box.max.x) * size, 
                                      cur_position.y + box.min.y * size,  
                                      p->current_layer);
            
            Painter__GoToNextLayer(p);
            
            Renderer_DrawTexturedQuad(p->render_commands,
                                      color,
                                      T*S*A,
                                      texture->handle,
                                      glyph->texture_uv_rect);
        }
        cur_position.x -= glyph->advance * size;
        if (index != 0) {
            u32 kerning = Font_GetKerning(font, str.data[index], str.data[index-1]);
            cur_position.x -= kerning * size;
        }
        
    }
}

static inline void
Painter_DrawTextRight(Painter* p,
					  Font_ID font_id,
					  Vec2f position,
					  Msg_ID msg_id,
					  f32 size, 
					  c4f color = C4F_WHITE) 
{
	String str = *(Assets_GetMsg(p->assets, msg_id));
	Painter_DrawTextRight(p, font_id, position, str, size, color);
}

// NOTE(Momo): Left aligned
static inline void 
Painter_DrawBigIntLeft(Painter* p,
                       Font_ID font_id,
                       Vec2f position,
                       BigInt* big_int,
                       f32 size, 
                       c4f color = C4F_WHITE) 
{
    Vec2f cur_position = position;
    Font* font = Assets_GetFont(p->assets, font_id);
    Texture* texture = Assets_GetTexture(p->assets, font->texture_id);
    
    for(u32 j = 0; j < big_int->count; ++j) {
        u32 i = big_int->count - 1 - j;
        Font_Glyph* glyph = Font_GetGlyph(font, DigitToAscii(big_int->data[i]));
        Rect2f box = glyph->glyph_box; 
        
        // NOTE(Momo): Set bottom left as origin
        Mat4f A = Mat_Translation(0.5f, 0.5f, 0.f); 
        
        f32 box_w = Rect_Width(box);
        f32 box_h = Rect_Height(box);
        
        Mat4f S = Mat_Scale(box_w * size, 
                            box_h* size, 
                            1.f);
        
        Mat4f T = Mat_Translation(cur_position.x + box.min.x * size, 
                                  cur_position.y + box.min.y * size,  
                                  p->current_layer);
        
        Painter__GoToNextLayer(p);
        
        Renderer_DrawTexturedQuad(p->render_commands,
                                  color,
                                  T*S*A,
                                  texture->handle,
                                  glyph->texture_uv_rect);
        
        cur_position.x += glyph->advance * size;
        if (i != big_int->count - 1 ) {
            u32 kerning = Font_GetKerning(font, 
                                          DigitToAscii(big_int->data[i]), 
                                          DigitToAscii(big_int->data[i+1]));
            cur_position.x += kerning * size;
        }
        
    }
}

static inline void 
Painter_DrawBigIntRight(Painter* p,
                        Font_ID font_id,
                        Vec2f position,
                        BigInt* big_int,
                        f32 size, 
                        c4f color = C4F_WHITE) 
{
    Vec2f cur_position = position;
    Font* font = Assets_GetFont(p->assets, font_id);
    Texture* texture = Assets_GetTexture(p->assets, font->texture_id);
    
    for(u32 i = 0; i < big_int->count; ++i) {
        Font_Glyph* glyph = Font_GetGlyph(font, DigitToAscii(big_int->data[i]));
        Rect2f box = glyph->glyph_box; 
        
        
        
        // NOTE(Momo): Set bottom right as origin
        Mat4f A = Mat_Translation(-0.5f, 0.5f, 0.f); 
        
        f32 box_w = Rect_Width(box);
        f32 box_h = Rect_Height(box);
        
        Mat4f S = Mat_Scale(box_w * size, 
                            box_h* size, 
                            1.f);
        
        Mat4f T = Mat_Translation(cur_position.x - (glyph->advance - box.max.x) * size, 
                                  cur_position.y + box.min.y * size,  
                                  p->current_layer);
        
        Painter__GoToNextLayer(p);
        
        Renderer_DrawTexturedQuad(p->render_commands,
                                  color,
                                  T*S*A,
                                  texture->handle,
                                  glyph->texture_uv_rect);
        
        cur_position.x -= glyph->advance * size;
        
        if (i != big_int->count - 1) {
            b32 kerning = Font_GetKerning(font, 
                                          DigitToAscii(big_int->data[i]), 
                                          DigitToAscii(big_int->data[i+1]));
            cur_position.x -= kerning * size;
        }
        
    }
}


#endif

