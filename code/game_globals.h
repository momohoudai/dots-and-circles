/* date = June 11th 2021 0:30 pm */

#ifndef GAME_GLOBALS_EXTERNAL_H
#define GAME_GLOBALS_EXTERNAL_H


// NOTE(Momo): Globals related to things from platform layer
Platform_API* g_platform;
Platform_Log* g_log;
Platform_Input* g_input;
Arena* g_scratch;


// NOTE(Momo): Game related globals
// Need to forward declare these
struct Assets;
Assets* g_assets;

struct Painter;
Painter* g_painter;

struct AudioMixer;
AudioMixer* g_mixer;



#endif // GAME_GLOBALS_H
