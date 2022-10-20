/* date = March 14th 2021 2:41 pm */

#ifndef TOOL_BUILD_ASSETS_ATLAS_H
#define TOOL_BUILD_ASSETS_ATLAS_H

//~ NOTE(Momo): Atlas builder
// [] In the future, we can consider lowering the memory footprint
// by figuring out what can be 'freed' after generating the texture.
// For example, we don't have to keep the fonts loaded.
// [] We should look into using String instead of (const char*) for filenames
// [] Cache images with same name?

struct AtlasBuilder__SubImageContext;

struct AtlasBuilder__Font {
    const char* filename; 
    stbtt_fontinfo info;
};

enum AtlasBuilder__EntryType {
    AtlasBuilder__EntryType_Image,
    AtlasBuilder__EntryType_Font,
};

struct AtlasBuilder__Entry_Font {
    Font_ID id;
    const char* filename;
    f32 raster_size; // font size to rasterize
    u32 codepoint_start;
    u32 codepoint_end;
    AtlasBuilder__Font* cached_font;
    
    Array<AtlasBuilder__SubImageContext> contexts;
};

struct AtlasBuilder__Entry_Image {
    Image_ID id;
    const char* filename;
    AtlasBuilder__SubImageContext* context;
};

struct AtlasBuilder__Entry {
    AtlasBuilder__EntryType type;
    union {
        AtlasBuilder__Entry_Font font;
        AtlasBuilder__Entry_Image image;
    };
};



enum AtlasBuilder__SubImageContextType {
    AtlasBuilder__SubImageContextType_Image,
    AtlasBuilder__SubImageContextType_FontGlyph,
};

struct AtlasBuilder__SubImageContext_Image {
    AtlasBuilder__Entry_Image* entry;
    
};

struct AtlasBuilder__SubImageContext_FontGlyph {
    AtlasBuilder__Entry_Font* entry;
    u32 codepoint;
};

struct AtlasBuilder__SubImageContext {
    AtlasBuilder__SubImageContextType type;
    Rect2u* corresponding_rect;
    union {
        AtlasBuilder__SubImageContext_FontGlyph font_glyph;
        AtlasBuilder__SubImageContext_Image image;
    };
};

struct AtlasBuilder {
    Arena arena; 
    
    // NOTE(Momo): atlas texture data
    Texture_ID texture_id;
    u32 width, height, channels;
    u8* texture;
    u32 sub_image_padding;
    
    // NOTE(Momo): Preparation-phase variables
    List<AtlasBuilder__Entry> entries;
    List<Rect2u> rects;
    List<AtlasBuilder__SubImageContext> contexts;
    
    // NOTE(Momo): font 'cache'
    // TODO: use hashmap instead?
    List<AtlasBuilder__Font> fonts;
};

static inline Memory_Block
AtlasBuilder__PushFileIntoMemory(Arena* arena, const char* filename) 
{
    Memory_Block ret = {};
    FILE* file = nullptr;
    fopen_s(&file, filename, "rb");
    
    if (file == nullptr) {
        return {};
    }
    defer { fclose(file); };
    
    
    fseek(file, 0, SEEK_END);
    u32 size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    void* buffer = Arena_PushBlock(arena, size);
    fread(buffer, size, 1, file);
    
    ret.data = buffer;
    ret.size = size;
    
    return ret;
}

static inline AtlasBuilder__Font*
AtlasBuilder__GetFontAndLoadItIfNotLoaded(AtlasBuilder* ab, const char* filename) {
    auto find_lamb = [](AtlasBuilder__Font* font, const char* filename) -> bool {
        return Sistr_Compare(font->filename, filename);
    };
    AtlasBuilder__Font* font = List_Find(&ab->fonts, find_lamb, filename);
    if (!font) {
        font = List_Push(&ab->fonts);
        Assert(font);
        Memory_Block file_memory = AtlasBuilder__PushFileIntoMemory(&ab->arena, filename);
        Assert(file_memory);
        b8 success = stbtt_InitFont(&font->info, (u8*)file_memory.data, 0);
        Assert(success);
        font->filename = filename;
    }
    
    return font;
}

static inline void 
AtlasBuilder_Begin(AtlasBuilder* ab, 
                   Texture_ID texture_id, 
                   u32 width, 
                   u32 height, 
                   u32 channels,
                   u32 sub_image_padding,
                   u32 entry_cap) 
{
    Arena_Clear(&ab->arena);
    
    
    ab->texture_id = texture_id;
    ab->width = width;
    ab->height = height;
    ab->channels = channels;
    ab->sub_image_padding = sub_image_padding; 
    
    b8 success = List_Alloc(&ab->entries, &ab->arena, entry_cap);
    Assert(success);
    
    
    
}

static inline void
AtlasBuilder_PushImage(AtlasBuilder* ab, Image_ID image_id, const char* filename) {
    // TODO(Momo): Maybe we check for duplicate filenames?
    // I don't really see why you would want two IDs
    // pointing to the same image? 
    // If we do, maybe we want to cache the image??
    // Sounds a little iffy to me...
    AtlasBuilder__Entry* entry = List_Push(&ab->entries);
    Assert(entry);
    entry->type = AtlasBuilder__EntryType_Image;
    entry->image.id = image_id;
    entry->image.filename = filename;
}

static inline void
AtlasBuilder_PushFont(AtlasBuilder* ab, Font_ID font_id, const char* filename, f32 raster_size, u32 codepoint_start, u32 codepoint_end ) 
{
    AtlasBuilder__Entry* entry = List_Push(&ab->entries);
    Assert(entry);
    entry->type = AtlasBuilder__EntryType_Font;
    entry->font.id = font_id;
    entry->font.filename = filename;
    entry->font.codepoint_start = codepoint_start;
    entry->font.codepoint_end = codepoint_end;
    entry->font.raster_size= raster_size;
    
    
}

static inline void 
AtlasBuilder__WriteSubTextureToAtlas(u8** atlas_memory, 
                                     u32 atlas_width, 
                                     u32 atlas_height,
                                     u8* texture_memory, 
                                     Rect2u texture_rect) 
{
    s32 j = 0;
    u32 texture_rect_w = Rect_Width(texture_rect);
    u32 texture_rect_h = Rect_Height(texture_rect);
    
    for (u32 y = texture_rect.min.y; y < texture_rect.min.y + texture_rect_h; ++y) {
        for (u32 x = texture_rect.min.x; x < texture_rect.min.x + texture_rect_w; ++x) {
            u32 Index = (x + y * atlas_width) * 4;
            Assert(Index < (atlas_width * atlas_height * 4));
            for (u32 c = 0; c < 4; ++c) {
                (*atlas_memory)[Index + c] = texture_memory[j++];
            }
        }
    }
}

// NOTE(Momo): Will generate image if filename is provided

static inline void
AtlasBuilder_End(AtlasBuilder* ab, const char* output_filename = nullptr) {
    // NOTE(Momo): Count the amount of rects, contexts and fonts
    // to figure out how much memory we need to reserve for them.
    u32 contexts_to_prepare = 0;
    u32 fonts_to_prepare = 0;
    {
        LinkedList<const char*> list = {};
        
        Arena_Marker temp_arena = Arena_Mark(&ab->arena);
        defer { Arena_Revert(&temp_arena); };
        
        for (u32 i = 0; i < ab->entries.count; ++i) {
            AtlasBuilder__Entry* entry = ab->entries + i;
            switch(entry->type) {
                case AtlasBuilder__EntryType_Image: {
                    ++contexts_to_prepare;
                } break;
                case AtlasBuilder__EntryType_Font: {
                    contexts_to_prepare += entry->font.codepoint_end - entry->font.codepoint_start + 1;
                    // search if font 
                    auto* itr = list.head;
                    b8 found = false;
                    while(itr) {
                        if(Sistr_Compare(itr->item, entry->font.filename)) {
                            found = true;
                            break;
                        }
                        itr = itr->next;
                    }
                    if (!found) {
                        ++fonts_to_prepare;
                    }
                } break;
            }
        }
    }
    
    
    // NOTE(Momo): Allocate memory for rects and contexts
    if (contexts_to_prepare == 0) {
        return;
    }
    else {
        b8 success = List_Alloc(&ab->rects, &ab->arena, contexts_to_prepare);
        Assert(success);
        success = List_Alloc(&ab->contexts, &ab->arena, contexts_to_prepare);
        Assert(success);
    }
    
    if (fonts_to_prepare > 0) {
        b8 success = List_Alloc(&ab->fonts, &ab->arena, fonts_to_prepare);
        Assert(success);
    }
    
    // NOTE(Momo): Load fonts
    for (u32 i = 0, j = 0; i < ab->entries.count; ++i) {
        AtlasBuilder__Entry* entry = ab->entries + i;
        if (entry->type == AtlasBuilder__EntryType_Font) {
            entry->font.cached_font = AtlasBuilder__GetFontAndLoadItIfNotLoaded(ab, entry->font.filename);
            
        }
    }
    
    
    // NOTE(Momo): assign rects and contexts
    for(u32 i = 0; i < ab->entries.count; ++i ) {
        AtlasBuilder__Entry* entry = ab->entries + i;
        switch(entry->type) {
            case AtlasBuilder__EntryType_Image: {
                AtlasBuilder__SubImageContext* context = List_Push(&ab->contexts);
                Assert(context);
                context->type = AtlasBuilder__SubImageContextType_Image;
                context->image.entry = &entry->image;
                
                s32 w, h, c;
                stbi_info(entry->image.filename, &w, &h, &c);
                
                Rect2u* rect = List_Push(&ab->rects);
                Assert(rect);
                (*rect) = Rect_CreateDims(Vec_Create((u32)w, (u32)h));
                context->corresponding_rect = rect;
                entry->image.context = context;
            } break;
            case AtlasBuilder__EntryType_Font: {
                for (u32 codepoint_index = entry->font.codepoint_start; codepoint_index <= entry->font.codepoint_end; ++codepoint_index) 
                {
                    AtlasBuilder__SubImageContext* context = List_Push(&ab->contexts);
                    Assert(context);
                    context->type = AtlasBuilder__SubImageContextType_FontGlyph;
                    context->font_glyph.entry = &entry->font;
                    context->font_glyph.codepoint = codepoint_index;
                    
                    f32 scale = stbtt_ScaleForPixelHeight(&entry->font.cached_font->info, entry->font.raster_size);
                    s32 ix0, iy0, ix1, iy1;
                    stbtt_GetCodepointTextureBox(&entry->font.cached_font->info, 
                                                 codepoint_index, 
                                                 scale,
                                                 scale, 
                                                 &ix0, &iy0, &ix1, &iy1);
                    Rect2u* rect = List_Push(&ab->rects);
                    Assert(rect);
                    (*rect) = Rect_CreateDims(Vec_Create((u32)(ix1 - ix0), (u32)(iy1 - iy0)));
                    
                    context->corresponding_rect = rect;
                }
                
                u32 context_count = entry->font.codepoint_end - entry->font.codepoint_start + 1;
                
                // Kinda hacky...?
                Array_Init(&entry->font.contexts, 
                           ab->contexts.data + ab->contexts.count - context_count, 
                           context_count);
                
            } break;
        } 
        
    }
    
    // NOTE(Momo): actual packing of rects
    if (!PackRects(&ab->arena,
                   ab->width,
                   ab->height,
                   ab->rects.data,
                   ab->rects.count, 
                   ab->sub_image_padding,
                   RectPacker_SortType_Height)) 
    {
        printf("[Atlas] Failed to generate texture\n");
        Assert(false);
    }
    
    // NOTE(Momo): Generate atlas from rects
    {
        u32 atlas_size = ab->width * ab->height * ab->channels;
        u8* atlas_memory = (u8*)Arena_PushBlock(&ab->arena, atlas_size);
        Assert(atlas_memory);
        
        for (u32 i = 0; i < ab->rects.count; ++i) {
            Rect2u rect = ab->rects[i];
            auto* context = ab->contexts + i;
            
            switch(context->type) {
                case AtlasBuilder__SubImageContextType_Image: {
                    Arena_Marker scratch = Arena_Mark(&ab->arena);
                    defer { Arena_Revert(&scratch); };
                    
                    s32 texture_w, texture_h, texture_c;
                    
                    Memory_Block file_mem = AtlasBuilder__PushFileIntoMemory(scratch,  context->image.entry->filename);
                    Assert(file_mem);
                    
                    // TODO: At the moment, there is no clean way for stbi load to 
                    // output to a given memory without some really ugly hacks. 
                    // so...oh well
                    //
                    // Github Issue: https://github.com/nothings/stb/issues/58          
                    //
                    u8* texture_memory = stbi_load_from_memory((u8*)file_mem.data, 
                                                               file_mem.size, 
                                                               &texture_w, &texture_h, &texture_c, 0);
                    
                    Assert((u32)texture_c == ab->channels);
                    
                    defer { stbi_image_free(texture_memory); };
                    AtlasBuilder__WriteSubTextureToAtlas(&atlas_memory, 
                                                         ab->width, 
                                                         ab->height, 
                                                         texture_memory, 
                                                         rect);
                    
                } break;
                case AtlasBuilder__SubImageContextType_FontGlyph: {
                    Arena_Marker scratch = Arena_Mark(&ab->arena);
                    defer { Arena_Revert(&scratch); };
                    
                    const u32 channels = ab->channels;
                    AtlasBuilder__Entry_Font* entry = context->font_glyph.entry;
                    
                    s32 w, h;
                    
                    f32 scale = stbtt_ScaleForPixelHeight(&entry->cached_font->info,
                                                          entry->raster_size);
                    u8* font_texture_one_ch = 
                        stbtt_GetCodepointTexture(&entry->cached_font->info, 
                                                  scale,
                                                  scale,
                                                  context->font_glyph.codepoint, 
                                                  &w, &h, nullptr, nullptr);
                    u32 texture_dim = (u32)(w * h);
                    // NOTE(Momo): some codepoints have no texture (eg. space)
                    if (texture_dim == 0) 
                        continue;
                    
                    Assert(font_texture_one_ch);
                    defer { stbtt_FreeTexture( font_texture_one_ch, nullptr ); };
                    
                    
                    
                    // TODO(Momo): Consider not using an arena to store
                    // a texture before writing to the atlas.
                    // We should be able to write directly into the atlas
                    // with font_texture_on_ch
                    u8* font_texture = (u8*)Arena_PushBlock(scratch, texture_dim*channels); 
                    Assert(font_texture);
                    defer { Arena_Revert(&scratch); };
                    
                    u8* font_texture_itr = font_texture;
                    for (u32 j = 0, k = 0; j < texture_dim; ++j ){
                        for (u32 l = 0; l < channels; ++l ) {
                            font_texture_itr[k++] = font_texture_one_ch[j];
                        }
                    }
                    AtlasBuilder__WriteSubTextureToAtlas(&atlas_memory, 
                                                         ab->width, 
                                                         ab->height, 
                                                         font_texture, 
                                                         rect);
                } break;
                
            }
            
        }
        
        ab->texture = atlas_memory;
    }
    
    // NOTE(Momo): output to a png file if filename is given
    if (output_filename) {
        stbi_write_png(output_filename, ab->width, ab->height, ab->channels, ab->texture, ab->width * ab->channels);
    }
    
}

#endif //TOOL_BUILD_ASSETS_ATLAS_H
