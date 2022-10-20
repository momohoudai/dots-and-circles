/* date = October 11th 2021 5:57 pm */

#ifndef WIN32_AUDIO_H
#define WIN32_AUDIO_H


struct Win32_Audio;
struct Win32_Audio__NotificationClient : IMMNotificationClient {
	LONG ref;
    Win32_Audio* audio;
	
	ULONG STDMETHODCALLTYPE AddRef()
    {
        return InterlockedIncrement(&ref);
    }
	
    ULONG STDMETHODCALLTYPE Release()
    {

        ULONG ulRef = InterlockedDecrement(&ref);
        if (0 == ulRef)
        {
            delete this;
        }
        return ulRef;
    }
	
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID **ppvInterface)
    {
        if (IID_IUnknown == riid)
        {
            AddRef();
            *ppvInterface = (IUnknown*)this;
        }
        else if (__uuidof(IMMNotificationClient) == riid)
        {
            AddRef();
            *ppvInterface = (IMMNotificationClient*)this;
        }
        else
        {
            *ppvInterface = NULL;
            return E_NOINTERFACE;
        }
        return S_OK;
    }
	HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow flow,
													 ERole role,
													 LPCWSTR pwstr_device_id);
	HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR pwstr_device_id) 
	{
		return S_OK;
	}
	
	HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR pwstr_device_id) 
	{
		return S_OK;
	}
	HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR pwstr_device_id, 
												   DWORD dwNewState) 
	{
		return S_OK;
	}
	HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR pwstr_device_id, 
													 const PROPERTYKEY key) 
	{
		return S_OK;
	}
};
struct Win32_Audio {
	
    // Wasapi
	Win32_Audio__NotificationClient notification_client;
	IMMDeviceEnumerator* device_enum;
    IAudioClient2* client;
    IAudioRenderClient* render_client;
    
    // "Secondary" buffer
    u32 buffer_size;
    s16* buffer;
    
    // Other variables for tracking purposes
    u32 latency_sample_count;
    u32 samples_per_second;
    u16 bits_per_sample;
    u16 channels;
    
	b8 is_device_changed;
	b8 is_device_ready;
};

HRESULT STDMETHODCALLTYPE 
Win32_Audio__NotificationClient::OnDefaultDeviceChanged(EDataFlow flow,
														ERole role,
														LPCWSTR pwstr_device_id) 
{
	Win32_Log("[Win32::Audio] Default device change detected\n");
	audio->is_device_changed = true;
	return S_OK;
}

static inline void 
Win32_Audio__FreeCurrentDevice(Win32_Audio* audio) {
	if (audio->client) {
		audio->client->Stop();
		audio->client->Release();
		audio->client = nullptr;
	}
	
	if (audio->render_client) {
		audio->render_client->Release();
		audio->render_client = nullptr;
	}
	audio->is_device_ready = false;
}


static inline void
Win32_Audio_Free(Win32_Audio* audio) {
	Win32_Audio__FreeCurrentDevice(audio);
	audio->device_enum->UnregisterEndpointNotificationCallback(&audio->notification_client);
	audio->device_enum->Release();
	Win32_FreeMemory(audio->buffer);
}

static inline b8
Win32_Audio__InitDefaultDevice(Win32_Audio* audio) 
{	
    IMMDevice* device;
    HRESULT hr = audio->device_enum->GetDefaultAudioEndpoint(eRender, 
															 eConsole, 
															 &device);
    if (FAILED(hr)) {
        Win32_Log("[Win32::Audio] Failed to get audio endpoint\n");
        return false;
    }
    defer { device->Release(); };
    
    hr = device->Activate(__uuidof(IAudioClient2), 
                          CLSCTX_ALL, 
                          nullptr, 
                          (LPVOID*)&audio->client);
    if(FAILED(hr)) {
		Win32_Log("[Win32::Audio] Failed to create IAudioClient\n");
        return false;
    }
    
    WAVEFORMATEX wave_format = {};
    wave_format.wFormatTag = WAVE_FORMAT_PCM;
    wave_format.wBitsPerSample = audio->bits_per_sample;
    wave_format.nChannels = audio->channels;
    wave_format.nSamplesPerSec = audio->samples_per_second;
    wave_format.nBlockAlign = (wave_format.nChannels * wave_format.wBitsPerSample / 8);
    wave_format.nAvgBytesPerSec = wave_format.nSamplesPerSec * wave_format.nBlockAlign;
    
    REFERENCE_TIME buffer_duration = 0;
    hr = audio->client->GetDevicePeriod(nullptr, &buffer_duration);
    
    DWORD stream_flags = ( AUDCLNT_STREAMFLAGS_RATEADJUST 
                          | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM
                          | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY );

    hr = audio->client->Initialize(AUDCLNT_SHAREMODE_SHARED, 
                                   stream_flags, 
                                   buffer_duration,
                                   0, 
                                   &wave_format, 
                                   nullptr);
    if (FAILED(hr))
    {
        Win32_Log("[Win32::Audio] Failed to initialize audio client\n");
        return false;
    }
    
    if (FAILED(audio->client->GetService(__uuidof(IAudioRenderClient),
                                         (LPVOID*)(&audio->render_client))))
    {
		Win32_Log("[Win32::Audio] Failed to create IAudioClient\n");
        return false;
    }
    
    UINT32 sound_frame_count;
    hr = audio->client->GetBufferSize(&sound_frame_count);
    if (FAILED(hr))
    {
        Win32_Log("[Win32::Audio] Failed to get buffer size\n");
        return false;
    }

    audio->buffer_size = sound_frame_count;
    audio->buffer = (s16*)Win32_AllocateMemory(audio->buffer_size);
    if (!audio->buffer) {
		Win32_Log("[Win32::Audio] Failed to allocate secondary buffer\n");
        return false;
    }
	
	
	
    // Win32_Log("[Win32::audio] Loaded!\n");
    
    audio->client->Start();
	audio->is_device_ready = true;
    return true;
}

static inline b8
Win32_Audio_Init(Win32_Audio* audio,
                 u32 samples_per_second, 
                 u16 bits_per_sample,
                 u16 channels,
                 u32 latency_frames,
                 u32 refresh_rate)
{
    audio->channels = channels;
    audio->bits_per_sample = bits_per_sample;
    audio->samples_per_second = samples_per_second;
    audio->latency_sample_count = (samples_per_second / refresh_rate) * latency_frames;
    
    HRESULT hr = CoInitializeEx(0, COINIT_SPEED_OVER_MEMORY);
    if (FAILED(hr)) {
        Win32_Log("[Win32::Audio] Failed CoInitializeEx\n");
        return false;
    }
    
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), 
                          nullptr,
                          CLSCTX_ALL, 
                          __uuidof(IMMDeviceEnumerator),
                          (LPVOID*)(&audio->device_enum));
    if (FAILED(hr)) {
        Win32_Log("[Win32::Audio] Failed to create IMMDeviceEnumerator\n");
		goto cleanup_1;
    }
    
	audio->notification_client.ref = 1;
	audio->notification_client.audio = audio;
	hr = audio->device_enum->RegisterEndpointNotificationCallback(&audio->notification_client);
	if(FAILED(hr)) {
		Win32_Log("[Win32::Audio] Failed to register notification callback\n");
		goto cleanup_2;
	}
	
	// NOTE(Momo): Allocate the maximum buffer possible given allowed latency
	audio->buffer_size = audio->latency_sample_count * sizeof(s16);
    audio->buffer = (s16*)Win32_AllocateMemory(audio->buffer_size);
    if (!audio->buffer) {
		Win32_Log("[Win32::Audio] Failed to allocate memory\n");
		goto cleanup_3;
    }
	
	Win32_Audio__InitDefaultDevice(audio);
	return true;
	
	// NOTE(Momo): Cleanup
	cleanup_3: {	
		Win32_FreeMemory(audio->buffer);
	}
	
	cleanup_2: {
		audio->device_enum->UnregisterEndpointNotificationCallback(&audio->notification_client);
	}
	
	cleanup_1: {
		audio->device_enum->Release();
	}
	return false;
		

	
}

static inline Platform_Audio
Win32_Audio_Begin(Win32_Audio* audio) {
	if (audio->is_device_changed) {
		Win32_Log("[Win32::Audio] Resetting audio device\n");
		// Attempt to change device
		Win32_Audio__FreeCurrentDevice(audio);
		Win32_Audio__InitDefaultDevice(audio);
		audio->is_device_changed = false;
	}
	
	
    Platform_Audio ret = {};
    
    UINT32 sound_padding_size;
    UINT32 samples_to_write = 0;
    
	if (audio->is_device_ready) {
		// Padding is how much valid data is queued up in the sound buffer
		// if there's enough padding then we could skip writing more data
		HRESULT hr = audio->client->GetCurrentPadding(&sound_padding_size);
		
		if (SUCCEEDED(hr)) {
			samples_to_write = (UINT32)audio->buffer_size - sound_padding_size;
			
			// Cap the samples to write to how much latency is allowed.
			if (samples_to_write > audio->latency_sample_count) {
				samples_to_write = audio->latency_sample_count;
			}
		}
	}
	else {
		// NOTE(Momo): if there is no device avaliable,
		// just write to the whole 'dummy' buffer.
		samples_to_write = audio->buffer_size;
	}
	
    ret.sample_buffer = audio->buffer;
    ret.sample_count = samples_to_write; 
    ret.channels = audio->channels;
    
    return ret;
}

static inline void
Win32_Audio_End(Win32_Audio* audio, 
				Platform_Audio output) 
{
	if (!audio->is_device_ready) {
		return;
	}
    // NOTE(Momo): Kinda assumes 16-bit Sound
    BYTE* sound_buffer_data;
    if (SUCCEEDED(audio->render_client->GetBuffer((UINT32)output.sample_count, &sound_buffer_data))) 
    {
        s16* src_sample = output.sample_buffer;
        s16* dest_sample = (s16*)sound_buffer_data;
        // buffer structure for stereo:
        // s16   s16    s16  s16   s16  s16
        // [LEFT RIGHT] LEFT RIGHT LEFT RIGHT....
        for(u32 sample_index = 0; sample_index < output.sample_count; ++sample_index){
            for (u32 channel_index = 0; channel_index < audio->channels; ++channel_index) {
                *dest_sample++ = *src_sample++;
            }
            
        }
        
        audio->render_client->ReleaseBuffer((UINT32)output.sample_count, 0);
    }
}

#endif //WIN32_AUDIO_H
