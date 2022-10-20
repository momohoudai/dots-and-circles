#ifndef TOOL_BUILD_ASSETS_H
#define TOOL_BUILD_ASSETS_H



//~ NOTE(Momo): Asset builder
struct AssetBuilder {
    Arena arena;
    
    FILE* file;
    u32 entry_count;
    long int entry_count_at;
    
};

static inline void
AssetBuilder_Begin(AssetBuilder* ab, const char* filename, const char* signature) 
{
    ab->entry_count = 0;
    ab->file = nullptr; 
    fopen_s(&ab->file, filename, "wb");
    Assert(ab->file && "Cannot open file");
    
    // NOTE(Momo): Write signature
    fwrite(signature, sizeof(u8), Sistr_Length(signature), ab->file);
    ab->entry_count_at = ftell(ab->file);
    
    // NOTE(Momo): Reserve space for entry_count
    fwrite(&ab->entry_count, sizeof(u32), 1, ab->file);
}

static inline void
AssetBuilder_End(AssetBuilder* ab) 
{
    fseek(ab->file, ab->entry_count_at, SEEK_SET);
    fwrite(&ab->entry_count, sizeof(u32), 1, ab->file);
    fclose(ab->file);
}

static inline void
AssetBuilder_WriteEntry(AssetBuilder* ab, Asset_Type asset_type) 
{
    Asset_File_Entry entry = {};
    entry.type = asset_type;
    fwrite(&entry, sizeof(entry),  1, ab->file);
    ++ab->entry_count;
}

static inline void 
AssetBuilder_WriteTexture(AssetBuilder* ab, 
                          Texture_ID id, 
                          u32 width, 
                          u32 height, 
                          u32 channels, 
                          u8* pixels) 
{
    AssetBuilder_WriteEntry(ab, Asset_Type_Texture);
    
    Asset_File_Texture texture = {};
    texture.id = id;
    texture.width = width;
    texture.height = height;
    texture.channels = channels;
    fwrite(&texture, sizeof(texture), 1, ab->file);
    
    
    u32 texture_size = width * height * channels;
    for(u32 i = 0; i < texture_size; ++i) {
        fwrite(pixels + i, 1, 1, ab->file);
    }
}


static inline void 
AssetBuilder_WriteTextureFromFile(AssetBuilder* ab, Texture_ID id, 
                                  const char* filename) 
{
    u32 width = 0, height = 0, channels = 0;
    u8* loaded_image = nullptr;
    {
        s32 w, h, c;
        loaded_image = stbi_load(filename, &w, &h, &c, 0);
        Assert(loaded_image != nullptr);
        
        width = (u32)w;
        height = (u32)h; 
        channels = (u32)c;
    }
    defer { stbi_image_free(loaded_image); };
    AssetBuilder_WriteTexture(ab, id, width, height, channels, loaded_image);
}

static inline Rect2f 
AssetBuilder__TexelUVToNormalizedUV(Vec2u texture_dims, Rect2u texel_uv) {
    
    Rect2f normalized_uv = Rect_Ratio(texel_uv, Rect_Create({}, texture_dims));
    
    // NOTE(Momo): Case where they 'cross', ie min is somehow less than max.
    // In this case, we will just 'make it simple' and set the uv to {0,0,0,0}
    if (normalized_uv.min.x < normalized_uv.max.x &&
        normalized_uv.min.y < normalized_uv.max.y) {
        return normalized_uv;
    }
    return {};
}

static inline void 
AssetBuilder__WriteAtlasImage(AssetBuilder* ass, 
                              AtlasBuilder* atlas,
                              AtlasBuilder__Entry_Image* img)
{
    AssetBuilder_WriteEntry(ass, Asset_Type_Image);
    
    Asset_File_Image file_image = {};
    file_image.id = img->id;
    file_image.texture_id = atlas->texture_id;
    file_image.texture_uv_rect = AssetBuilder__TexelUVToNormalizedUV(Vec_Create(atlas->width, atlas->height),
                                                                     (*img->context->corresponding_rect));
    fwrite(&file_image, sizeof(file_image), 1, ass->file);
}

static inline void
AssetBuilder__WriteAtlasFont(AssetBuilder* ab, 
                             AtlasBuilder* atlas,
                             AtlasBuilder__Entry_Font* font) 
{
    AssetBuilder_WriteEntry(ab, Asset_Type_Font);
    stbtt_fontinfo* font_info = &font->cached_font->info;
    
    s32 ascent, descent, line_gap;
    stbtt_GetFontVMetrics(font_info, &ascent, &descent, &line_gap); 
    
    f32 pixel_scale = stbtt_ScaleForPixelHeight(&font->cached_font->info, 1);
    
    Asset_File_Font file_font = {};
    file_font.id = font->id;
    file_font.ascent = ascent * pixel_scale;
    file_font.descent = descent * pixel_scale;
    file_font.line_gap = line_gap * pixel_scale;
    file_font.codepoint_start = font->codepoint_start;
    file_font.codepoint_end = font->codepoint_end;
    file_font.texture_id = atlas->texture_id;
    fwrite(&file_font, sizeof(file_font), 1, ab->file);
    
    // Kerning
    for (u32 i = font->codepoint_start; i <= font->codepoint_end; ++i) {
        for (u32 j = font->codepoint_start; j <= font->codepoint_end; ++j) {
            s32 kerning = stbtt_GetCodepointKernAdvance(&font->cached_font->info, (s32)i, (s32)j);
            
            Asset_File_Font_Kerning font_kerning = {};
            font_kerning.kerning = kerning; 
            font_kerning.codepoint_a = i;
            font_kerning.codepoint_b = j;
            fwrite(&font_kerning, sizeof(font_kerning), 1, ab->file);
        }
    }
    
    // Glyphs
    for(u32 i = 0; i < font->contexts.count; ++i) {
        AtlasBuilder__SubImageContext* context = font->contexts + i;
        Asset_File_Font_Glyph font_glyph = {};
        s32 advance;
        s32 left_side_bearing; 
        u32 codepoint = context->font_glyph.codepoint;
        font_glyph.texture_uv_rect = AssetBuilder__TexelUVToNormalizedUV(Vec_Create(atlas->width, atlas->height),
                                                                         (*context->corresponding_rect));
        
        stbtt_GetCodepointHMetrics(font_info, 
                                   codepoint, 
                                   &advance, 
                                   &left_side_bearing);
        
        Rect2s box;
        stbtt_GetCodepointBox(font_info, 
                              codepoint, 
                              &box.min.x, 
                              &box.min.y, 
                              &box.max.x, 
                              &box.max.y);
        
        font_glyph.codepoint = codepoint;
        font_glyph.advance = advance * pixel_scale;
        font_glyph.glyph_box = Rect_Scale(Rect_Cast<f32>(box), pixel_scale);
        
        fwrite(&font_glyph, sizeof(font_glyph), 1, ab->file);
    }
}


static inline void 
AssetBuilder_WriteAnime(AssetBuilder* ab, 
                        Anime_ID anime_id, 
                        Image_ID* frames,
                        u32 frame_count) 
{
    AssetBuilder_WriteEntry(ab, Asset_Type_Anime);
    
    Asset_File_Anime anime = {};
    anime.id = anime_id;
    anime.frame_count = frame_count;
    
    fwrite(&anime, sizeof(anime), 1, ab->file);
    fwrite(frames, sizeof(Image_ID), frame_count, ab->file);
}

static inline void 
AssetBuilder_WriteMsg(AssetBuilder* ab, Msg_ID msg_id,
                      const char* message) 
{
    AssetBuilder_WriteEntry(ab, Asset_Type_Msg);
    
    Asset_File_Msg msg = {};
    msg.id = msg_id;
    msg.count = Sistr_Length(message);
    
    fwrite(&msg, sizeof(msg), 1, ab->file);
    fwrite(message, sizeof(char), msg.count, ab->file);
}


// NOTE(Momo): Right now we only support WAV
static inline void
AssetBuilder_WriteSound(AssetBuilder* ab, 
                        Sound_ID sound_id, 
                        const char* filename) 
{
    Arena_Marker mark = Arena_Mark(&ab->arena);
    defer { Arena_Revert(&mark); };
    
    Memory_Block file_memory = ReadFileIntoMemory(filename, mark);
    Assert(file_memory && "Cannot read file");
    
    WAV wav_result = WAV_LoadFromMemory(file_memory.data,
                                        file_memory.size);
    Assert(wav_result);
    
    // We restrict the type of sound the game allows here
    Assert(wav_result.fmt_chunk.num_channels == GAME_AUDIO_CHANNELS);
    Assert(wav_result.fmt_chunk.sample_rate == GAME_AUDIO_SAMPLES_PER_SECOND);
    Assert(wav_result.fmt_chunk.bits_per_sample == GAME_AUDIO_BITS_PER_SAMPLE);
    
    AssetBuilder_WriteEntry(ab, Asset_Type_Sound);
    
    Asset_File_Sound sound = {};
    sound.id = sound_id;
    sound.data_count = wav_result.data_chunk.size / sizeof(s16);
    
    fwrite(&sound, sizeof(sound), 1, ab->file);
    fwrite(wav_result.data, sizeof(s16), sound.data_count, ab->file);
    
}

static inline void
AssetBuilder_WriteAtlas(AssetBuilder* ass, AtlasBuilder* atlas) {
    // NOTE(Momo): We assume that atlas is 'ready'.
    
    // NOTE(Momo): Write the texture
    AssetBuilder_WriteTexture(ass, atlas->texture_id, atlas->width, atlas->height, atlas->channels, atlas->texture);
    
    // NOTE(Momo): Write the images
    // NOTE(Momo): Write the font + font_glyph + kerning
    
    for (u32 i = 0; i < atlas->entries.count; ++i) {
        AtlasBuilder__Entry* entry = atlas->entries + i;
        switch(entry->type) {
            case AtlasBuilder__EntryType_Image: {
                AssetBuilder__WriteAtlasImage(ass, atlas, &entry->image);
            } break;
            case AtlasBuilder__EntryType_Font: {
                AssetBuilder__WriteAtlasFont(ass, atlas, &entry->font);
            } break;
        }
    }
    
}

#endif //TOOL_BUILD_ASSETS_H
