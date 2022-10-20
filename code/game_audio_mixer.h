/* date = May 4th 2021 4:50 pm */

#ifndef GAME_AUDIO_MIXER_H
#define GAME_AUDIO_MIXER_H

// TODO(Momo): 
// - A pool of sound counters will reference a sound asset and 'play' it 
// - Output result to buffer, or we can just grab a handle to the game_audio struct
//   and manipulate the audio buffer directly?

struct AudioMixer_Instance {
    Sound_ID sound_id; 
    u32 current_offset; // Current offset of the sound data
    u32 index;
    
    b8 is_loop;
    b8 is_playing;
    f32 volume;
    
};


struct AudioMixer {
    Array<AudioMixer_Instance> instances;
    List<u32> free_list;
    f32 volume;
};

static inline b8
AudioMixer_Init(AudioMixer* a,
                f32 master_volume,
                u32 max_instances,
                Arena* arena) 
{
    // NOTE(Momo): We reject the highest value because
    // we are reserving one index to represent an invalid index
    // which is index == 0
    if (max_instances == 0 || max_instances == U32_MAX) {
        return false;
    }
    Arena_Marker mark = Arena_Mark(arena);
    b8 success = Array_Alloc(&a->instances, arena, max_instances);
    if (!success) {
        Arena_Revert(&mark);
        return false;
    }
    
    success = List_Alloc(&a->free_list, arena, max_instances);
    if (!success) {
        Arena_Revert(&mark);
        return false;
    }
    a->volume = master_volume;
    
    for (u32 i = 0; i < a->instances.count; ++i ){
        a->instances[i] = {0};
        u32* item = List_Push(&a->free_list);
        if(!item) {
            return false;
        }
        (*item) = i;
    }
    
    return true;
}

static inline AudioMixer_Instance*
AudioMixer_Play(AudioMixer* a, 
                Sound_ID sound_id, 
                b8 loop,
                f32 volume = 1.f)
{
    u32* index_ptr = List_Last(&a->free_list);
    
    if (index_ptr == nullptr) 
        return nullptr;
    
    List_Pop(&a->free_list);
    u32 index = (*index_ptr);
    
    AudioMixer_Instance* instance = a->instances + index;
    instance->is_loop = loop;
    instance->current_offset = 0;
    instance->sound_id = sound_id;
    instance->is_playing = true;
    instance->volume = volume;
    instance->index = index;
    
    return instance;
}


static inline void
AudioMixer_Stop(AudioMixer* a, AudioMixer_Instance* instance) 
{
    Assert(instance->index < a->instances.count);
#if INTERNAL
    // NOTE(Momo): Check if index already exist in free list
    for (u32 i = 0; i < a->free_list.count; ++i) {
        Assert(instance->index != a->free_list[i]);
    }
#endif
    
    instance->is_playing = false;
    List_PushItem(&a->free_list, instance->index);
}


static inline void
AudioMixer_Update(AudioMixer* a, Platform_Audio* audio) 
{
#if 1
    s16* sample_out = audio->sample_buffer;
    for(u32 sample_index = 0; sample_index < audio->sample_count; ++sample_index) {
        
        for (u32 channel_index = 0; channel_index < audio->channels; ++channel_index) {
            sample_out[channel_index] = 0;
        }
        
        for (u32 instance_index = 0; instance_index < a->instances.count; ++instance_index) {
            AudioMixer_Instance* instance = a->instances + instance_index;
            if (instance->is_playing == false) {
                continue;
            }
            Sound* sound = Assets_GetSound(g_assets, instance->sound_id);
            
            for (u32 channel_index = 0; channel_index < audio->channels; ++channel_index) {
                sample_out[channel_index] += s16(sound->data[instance->current_offset++] * 
                                     a->volume * instance->volume);
            }
            
            if (instance->current_offset >= sound->data_count) {
                if (instance->is_loop) {
                    instance->current_offset = 0;
                }
                else {
                    AudioMixer_Stop(a, instance);
                }
            }
            
            
            
        }
        sample_out += audio->channels;
        
    }
#else // This is for testing only
  static f32 sine = 0.f;
  s16* sample_out = audio->sample_buffer;
  s16 volume = 3000;
  for(u32 sample_index = 0; sample_index < audio->sample_count; ++sample_index) {
      for (u32 channel_index = 0; channel_index < audio->channels; ++channel_index) {
        f32 sine_value = sinf(sine);
        sample_out[channel_index] = s16(sine_value * volume);
      }
      sample_out += audio->channels;
      sine += 2.f;
  }
#endif
}


#endif //GAME_AUDIO_MIXER_H
