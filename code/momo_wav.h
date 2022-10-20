/* date = March 14th 2021 2:49 pm */
#ifndef MM_WAV_H
#define MM_WAV_H


// NOTE(Momo): http://soundfile.sapp.org/doc/Waveformat/
struct WAV_Riff_Chunk {
    u32 id; // big endian
    u32 size;
    u32 format; // big endian
};



struct WAV_Fmt_Chunk {
    u32 id;
    u32 size;
    u16 audio_format;
    u16 num_channels;
    u32 sample_rate;
    u32 byte_rate;
    u16 block_align;
    u16 bits_per_sample;
};

struct WAV_Data_Chunk {
    u32 id;
    u32 size;
};

struct WAV {
    WAV_Riff_Chunk riff_chunk;
    WAV_Fmt_Chunk fmt_chunk;
    WAV_Data_Chunk data_chunk;
    void* data;
    
    inline operator bool() {
        return data != nullptr;
    }
};

// NOTE(Momo): Will actually leave data into arena
static inline WAV 
WAV_LoadFromMemory(void* memory, 
                   u32 memory_size) 
{
    WAV ret = {};
    
    constexpr static u32 riff_id_signature = 0x52494646;
    constexpr static u32 riff_format_signature = 0x57415645;
    constexpr static u32 fmt_id_signature = 0x666d7420;
    constexpr static u32 data_id_signature = 0x64617461;
    
    Stream stream = {};
    if (!Stream_Init(&stream, memory, memory_size)) {
        return {};
    }
    
    // NOTE(Momo): Load Riff Chunk
    auto* riff_chunk = Stream_Consume<WAV_Riff_Chunk>(&stream);
    if (!riff_chunk) {
        return {};
    }
    riff_chunk->id = EndianSwapU32(riff_chunk->id);
    riff_chunk->format = EndianSwapU32(riff_chunk->format);
    if (riff_chunk->id != riff_id_signature) {
        return {};
    }
    if (riff_chunk->format != riff_format_signature) {
        return {};
    }
    
    // NOTE(Momo): Load fmt Chunk
    auto* fmt_chunk = Stream_Consume<WAV_Fmt_Chunk>(&stream);
    if (!fmt_chunk) {
        return {};
    }
    fmt_chunk->id = EndianSwapU32(fmt_chunk->id);
    if (fmt_chunk->id != fmt_id_signature) {
        return {};
    }
    if (fmt_chunk->size != 16) {
        return {};
    }
    if (fmt_chunk->audio_format != 1) {
        return {};
    }
    
    u32 bytes_per_sample = fmt_chunk->bits_per_sample/8;
    if (fmt_chunk->byte_rate != 
        fmt_chunk->sample_rate * fmt_chunk->num_channels * bytes_per_sample) {
        return {};
    }
    if (fmt_chunk->block_align != fmt_chunk->num_channels * bytes_per_sample) {
        return {};
    }
    
    // NOTE(Momo): Load data Chunk
    auto* data_chunk = Stream_Consume<WAV_Data_Chunk>(&stream);
    if (!data_chunk) {
        return {};
    }
	data_chunk->id = EndianSwapU32(data_chunk->id);
    if (data_chunk->id != data_id_signature) {
        return {};
    }
    
    void* data = Stream_ConsumeBlock(&stream, data_chunk->size);
    if (data == nullptr) {
        return {};
    }
    
    ret.riff_chunk = (*riff_chunk);
    ret.fmt_chunk = (*fmt_chunk);
    ret.data_chunk = (*data_chunk);
    ret.data = data;
    
    return ret;
}

#endif //MM_WAV_H
