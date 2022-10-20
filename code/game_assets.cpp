

//~ NOTE(Momo): Font functions
static inline f32 
Font_Height(Font* f) {
    return Abs(f->ascent) + Abs(f->descent);
}

static inline u32
Font_CodepointCount(Font* f) {
    return f->codepoint_end - f->codepoint_start + 1;
}

static inline u32
Font_GetKerning(Font* f, u32 codepoint_a, u32 codepoint_b) {
    if (codepoint_a < f->codepoint_start ||
        codepoint_a > f->codepoint_end ||
        codepoint_b < f->codepoint_start ||
        codepoint_a > f->codepoint_end) {
        
        return 0;
    }
    u32 index_a = codepoint_a - f->codepoint_start;
    u32 index_b = codepoint_b - f->codepoint_start;
    u32 ret = f->kernings[index_a + index_b * Font_CodepointCount(f)];
    
    return ret;
}

static inline Font_Glyph* 
Font_GetGlyph(Font* f, u32 codepoint) {
    if (codepoint < f->codepoint_start ||
        codepoint > f->codepoint_end) {
        return nullptr;
    }
    
    u32 index = codepoint - f->codepoint_start;
    Font_Glyph* ret = f->glyphs + index;
    
    return ret;
}


//~ NOTE(Momo): Asset functions
static inline Font*
Assets_GetFont(Assets* a, Font_ID font_id) {
    Assert(font_id < Font_ID_Count);
    return a->fonts + font_id;
}

static inline Texture*
Assets_GetTexture(Assets* a, Texture_ID texture_id) {
    Assert(texture_id < Texture_ID_Count);
    return a->textures + texture_id;
}

static inline Image*
Assets_GetImage(Assets* a, Image_ID image_id) {
    Assert(image_id < Image_ID_Count);
    return a->images + image_id;
}

static inline Anime*
Assets_GetAnime(Assets* a, Anime_ID anime_id) {
    Assert(anime_id < Anime_ID_Count);
    return a->animes + anime_id;
}


static inline Msg*
Assets_GetMsg(Assets* a, Msg_ID msg_id) {
    Assert(msg_id < Msg_ID_Count);
    return a->msgs + msg_id;
}


static inline Sound*
Assets_GetSound(Assets* a, Sound_ID sound_id) {
    Assert(sound_id < Sound_ID_Count);
    return a->sounds + sound_id;
}


static inline b8
Assets__CheckSignature(void* memory, String signature) {
    u8* memory_u8 = (u8*)memory;
    for (u32 i = 0; i < signature.count; ++i) {
        if (memory_u8[i] != signature.data[i]) {
            return false;
        }
    }
    return true; 
}



static inline b8
Assets_Init(Assets* a, Arena* arena, Arena* temp_arena) 
{
	Platform_File_Handle asset_file = g_platform->open_file(GAME_ASSET_FILENAME, 
                                                            Platform_File_Open_Mode_Read,
															Platform_File_Location_Game);
    if (asset_file.error) {
        g_platform->log_file_error(&asset_file);
        return false;
    }
    defer { g_platform->close_file(&asset_file); }; 
	
	Arena_Clear(arena);
	
    a->texture_count = Texture_ID_Count;
    a->textures = Arena_PushArray<Texture>(arena, Texture_ID_Count);
    
    a->image_count = Image_ID_Count;
    a->images = Arena_PushArray<Image>(arena, Image_ID_Count);
    
    a->font_count = Font_ID_Count;
    a->fonts = Arena_PushArray<Font>(arena, Font_ID_Count);
    
    a->anime_count = Anime_ID_Count;
    a->animes = Arena_PushArray<Anime>(arena, Anime_ID_Count);
    
    a->msg_count = Msg_ID_Count;
    a->msgs = Arena_PushArray<Msg>(arena, Msg_ID_Count);
    
    a->sound_count = Sound_ID_Count;
    a->sounds = Arena_PushArray<Sound>(arena, Sound_ID_Count);
	
    g_platform->clear_textures();
	
    
    u32 cur_file_offset = 0;
    u32 file_entry_count = 0;
    
    // Check file signaure
    {        
        defer { Arena_Clear(temp_arena); };
        
        String signature = String_Create(GAME_ASSET_FILE_SIGNATURE);
        
        void* read_sig = Platform_ReadBlock(g_platform,
                                            &asset_file,
                                            temp_arena,
                                            &cur_file_offset,
                                            (u32)signature.count,
                                            1);
        if (read_sig == nullptr) {
            g_log("[Assets] Cannot read signature\n");
            return false;
        }
        
        if (!Assets__CheckSignature(read_sig, signature)) {
            g_log("[Assets] Wrong asset signature\n");
            return false;
        }
        
        // Get File Entry
        u32* file_entry_count_ptr = Platform_Read<u32>(g_platform,
                                                       &asset_file,
                                                       temp_arena,
                                                       &cur_file_offset);
        if (file_entry_count_ptr == nullptr) {
            g_log("[Assets] Cannot get file entry count\n");
            return false;
        }
        file_entry_count = *file_entry_count_ptr;
    }
    
    
    for (u32 i = 0; i < file_entry_count; ++i)  {
        // NOTE(Momo): Read header
        Asset_File_Entry file_entry = {};
        {
            defer { Arena_Clear(temp_arena); };
            
            auto* file_entry_ptr = Platform_Read<Asset_File_Entry>(g_platform,
                                                                   &asset_file,
                                                                   temp_arena,
                                                                   &cur_file_offset);
            if (file_entry_ptr == nullptr) {
                g_log("[Assets] Cannot get file entry\n");
                return false;
            }
            file_entry = *file_entry_ptr;
        }
        
        switch(file_entry.type) {
            case Asset_Type_Texture: {
                defer { Arena_Clear(temp_arena); };
                
                auto* file_texture = Platform_Read<Asset_File_Texture>(g_platform,
                                                                       &asset_file,
                                                                       temp_arena,
                                                                       &cur_file_offset);              
                if (file_texture == nullptr) {
                    g_log("[Assets] Error getting texture\n");
                    return false;
                }
                auto* texture = a->textures + file_texture->id;
                texture->width = file_texture->width;
                texture->height = file_texture->height;
                texture->channels = file_texture->channels;
                u32 texture_size = texture->width * 
				texture->height * 
				texture->channels;
                
                texture->data = (u8*)Platform_ReadBlock(g_platform,
                                                        &asset_file, 
                                                        arena, 
                                                        &cur_file_offset,
                                                        texture_size,
                                                        1);
                if (texture->data == nullptr) {
                    g_log("[Assets] Error getting texture pixels\n");
                    return false;
                }
                texture->handle = g_platform->add_texture(file_texture->width, 
                                                          file_texture->height,
                                                          texture->data);
                if (!texture->handle.success) {
                    g_log("[Assets] Cannot add texture!");
                    return false;
                }
            } break;
            case Asset_Type_Image: { 
                defer { Arena_Clear(temp_arena); };
                
                auto* file_image = 
				Platform_Read<Asset_File_Image>(g_platform,
												&asset_file, 
												temp_arena,
												&cur_file_offset);              
                
                if (file_image == nullptr) {
                    g_log("[Assets] Error getting image\n");
                    return false;
                }
                
                auto* image = Assets_GetImage(a, file_image->id);
                image->texture_uv_rect = file_image->texture_uv_rect;
                image->texture_id = file_image->texture_id;
            } break;
            case Asset_Type_Font: {
                defer { Arena_Clear(temp_arena); };
                
                
                auto* file_font = Platform_Read<Asset_File_Font>(g_platform,
                                                                 &asset_file,
                                                                 temp_arena,
                                                                 &cur_file_offset);
                if (file_font == nullptr) {
                    g_log("[Assets] Error getting font\n");
                    return false;
                }
                
                Font* font = a->fonts + file_font->id;
                font->line_gap = file_font->line_gap;
                font->ascent = file_font->ascent;
                font->descent = file_font->descent;
                font->codepoint_start = file_font->codepoint_start;
                font->codepoint_end = file_font->codepoint_end;
                font->texture_id = file_font->texture_id;
                
                // Kerning
                u32 codepoint_count = Font_CodepointCount(font);
                font->kernings = Arena_PushArray<u32>(arena, codepoint_count * codepoint_count);
                for (u32 kerning_i = 0; kerning_i < codepoint_count; ++kerning_i) {
                    for (u32 kerning_j = 0; kerning_j < codepoint_count; ++kerning_j) {
                        Arena_Marker kernintemp_arena = Arena_Mark(temp_arena);
                        defer { Arena_Revert(&kernintemp_arena); };
                        
                        auto* single_kerning_data = Platform_Read<Asset_File_Font_Kerning>(g_platform, &asset_file, kernintemp_arena, &cur_file_offset);
                        if (single_kerning_data == nullptr) {
                            g_log("[Assets] Error getting kerning\n");
                            return false;
                        }
                        
                        u32 codepoint_a = single_kerning_data->codepoint_a;
                        u32 codepoint_b = single_kerning_data->codepoint_b;
                        if (codepoint_a < font->codepoint_start ||
                            codepoint_a > font->codepoint_end ||
                            codepoint_b < font->codepoint_start ||
                            codepoint_a > font->codepoint_end) {
                            g_log("[Assets] Invalid codepoint for kerning. Start: %d, End: %d, codepoint_a: %d, codepoint_b: %d\n", font->codepoint_start, font->codepoint_end, codepoint_a, codepoint_b);
                            return false;
                        }
                        
                        
                        u32 index_i = codepoint_a - font->codepoint_start;
                        u32 index_j = codepoint_b - font->codepoint_start;
                        font->kernings[index_j + index_i * Font_CodepointCount(font)] = single_kerning_data->kerning;
                        
                        
                    }
                }
                
                // glyph
                font->glyphs = Arena_PushArray<Font_Glyph>(arena, codepoint_count);
                for(u32 glyph_i = 0; glyph_i < codepoint_count; ++glyph_i) {
                    Arena_Marker glyph_scratch = Arena_Mark(temp_arena);
                    defer { Arena_Revert(&glyph_scratch); };
                    
                    auto* single_glyph_data = Platform_Read<Asset_File_Font_Glyph>(g_platform, &asset_file, glyph_scratch, &cur_file_offset);
                    if (single_glyph_data == nullptr) {
                        g_log("[Assets] Error getting glyph\n");
                        return false;
                    }
                    if (single_glyph_data->codepoint < font->codepoint_start ||
                        single_glyph_data->codepoint > font->codepoint_end) {
                        g_log("[Assets] Invalid codepoint for glyph. Start: %d, End: %d, codepoint: %d\n", font->codepoint_start, font->codepoint_end, single_glyph_data->codepoint);
                        return false;
                    }
                    u32 index = single_glyph_data->codepoint - font->codepoint_start;
                    
                    Font_Glyph* glyph = font->glyphs + index;
                    glyph->texture_uv_rect = single_glyph_data->texture_uv_rect;
                    glyph->glyph_box = single_glyph_data->glyph_box;
                    glyph->advance = single_glyph_data->advance;
                    
                    
                }
                
                
            } break;
            case Asset_Type_Sound: {
                defer { Arena_Clear(temp_arena); };
                
                auto* file = Platform_Read<Asset_File_Sound>(g_platform,
                                                             &asset_file,
                                                             temp_arena,
                                                             &cur_file_offset);
                
                if (file == nullptr) { 
                    g_log("[Assets] Error getitng sound\n"); 
                    return false; 
                }
                
                Sound* sound = Assets_GetSound(a, file->id);
                sound->data_count = file->data_count;
                sound->data = (s16*)Platform_ReadBlock(g_platform,
                                                       &asset_file,
                                                       arena, 
                                                       &cur_file_offset,
                                                       sizeof(s16) * sound->data_count,
                                                       1);
                
            } break;
            case Asset_Type_Msg: {
                defer { Arena_Clear(temp_arena); };
                auto* file = Platform_Read<Asset_File_Msg>(g_platform,
                                                           &asset_file,
                                                           temp_arena,
                                                           &cur_file_offset);
                if (file == nullptr) { 
                    g_log("[Assets] Msg is null"); 
                    return false; 
                }
                
                
                Msg* msg = Assets_GetMsg(a, file->id);
                msg->count = file->count;
                
                msg->data = (u8*)Platform_ReadBlock(g_platform,
                                                    &asset_file,
                                                    arena, 
                                                    &cur_file_offset,
                                                    sizeof(u8) * msg->count,
                                                    1);
                
                
                
            } break;
            case Asset_Type_Anime: {
                defer { Arena_Clear(temp_arena); };
                auto* file = Platform_Read<Asset_File_Anime>(g_platform,
                                                             &asset_file,
                                                             temp_arena,
                                                             &cur_file_offset);
                
                if (file == nullptr) { 
                    g_log("[Assets] Anime is null"); 
                    return false; 
                }
                
                
                Anime* anime = Assets_GetAnime(a, file->id);
                anime->frame_count = file->frame_count;
                
                anime->frames = (Image_ID*)Platform_ReadBlock(g_platform,
                                                              &asset_file,
                                                              arena, 
                                                              &cur_file_offset,
                                                              sizeof(Image_ID) * anime->frame_count,
                                                              1);
                
            } break;
            default: {
                return false;
            } break;
            
            
        }
    }
    
    
    return true;
    
}
