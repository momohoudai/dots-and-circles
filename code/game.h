#ifndef GAME_H
#define GAME_H

#include "momo.h"


// NOTE(Momo): To be included and used by platform/renderer layer
#include "game_config.h"
#include "game_renderer.h"
#include "game_platform.h"


// NOTE(Momo): To be used only by game layer
#include "game_globals.h"
#include "game_profiler.h"
#include "game_assets_file.h"
#include "game_assets.h"

#include "game_draw.h"
#include "game_audio_mixer.h"


#if INTERNAL
#include "game_debug_inspector.h"
#include "game_debug_console.h"
#endif // INTERNAL

#include "game_mode.h"

#include "game_mode_menu.h"
#include "game_mode_main.h"
#include "game_mode_main.cpp"
#include "game_commands.h"




#endif //GAME_H
