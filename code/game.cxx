#include "game.h"

#if INTERNAL
// cmd: jump main/menu/atlas_test/etc...
static inline void 
CmdJump(Console* console, void* context, String args) {
    auto* perm_state = (Permanent_State*)context;
    
    defer{ Arena_Clear(g_scratch); };
    
    String buffer = {};
    String_Split_Result arg_list = String_Split(args, g_scratch, ' ');
    if ( arg_list.item_count != 2 ) {
        // Expect two arguments
        buffer = String_Create("Expected only 2 arguments");
        Console_PushInfo(console, buffer, C4F_RED);
        return;
    }
    
    String state_to_chage_to = arg_list.items[1];
    if (state_to_chage_to == "main") {
        buffer = String_Create("Jumping to Main");
        Console_PushInfo(console, buffer, C4F_YELLOW);
        perm_state->next_game_mode = Game_Mode_Type_Main;
    }
    else if (state_to_chage_to == "menu") {
        buffer = String_Create("Jumping to Menu");
        Console_PushInfo(console, buffer, C4F_YELLOW);
        perm_state->next_game_mode = Game_Mode_Type_Menu;
    }
    
    else {
        buffer = String_Create("Invalid state to jump to");
        Console_PushInfo(console, buffer, C4F_RED);
    }
}


// cmd: reset
// resets current state
static inline void 
CmdReset(Console* console, void* context, String args) {
    auto* perm_state = (Permanent_State*)context;
    
    perm_state->next_game_mode = perm_state->current_game_mode;
    String buffer = String_Create("Resetting State");
    Console_PushInfo(console, buffer, C4F_YELLOW);
}
#endif // INTERNAL

//~ NOTE(Momo): Exported functions


extern "C" 
Game_DeclareGameUpdateFunction(Game_Update) 
{
    // NOTE(Momo): Initialize globals
    
    Painter painter = {};
    {
        // Let's say we want to time this block
        g_platform = platform_api;
        g_log = platform_api->log;
        g_input = platform_input;
        
        g_painter = &painter;
    }
    
    auto* perm_state = (Permanent_State*)game_memory->permanent_memory;
    
    //- NOTE(Momo)  Initialization of the game
    // Maybe we should move this to a 'init' function and make the platform layer
    // call it but ehhhhhhh seems like a lot of work for no real benefit.
    if(!perm_state->is_initialized) {
        g_log("[Game] Init Begin\n");
        
        
        //- NOTE(Momo): Arenas
        perm_state = Arena_Boot(Permanent_State,
                                arena,
                                game_memory->permanent_memory, 
                                game_memory->permanent_memory_size);
        
        //- NOTE(Momo): Audio
        b8 success = AudioMixer_Init(&perm_state->mixer, 1.f, 32, &perm_state->arena);
        if (!success) {
            g_log("[Game] Failed to initialize audio\n");
            return false;
        }
        
#if INTERNAL
        //- NOTE(Momo): Inspector
        if (!Inspector_Init(&perm_state->inspector, &perm_state->arena)){
            return false;
        }
        
        
        //- NOTE(Momo): Console
        {
            if (!Console_Init(&perm_state->console, &perm_state->arena, 16)) {
                g_log("[Game] Failed to initialize console\n");
                return false;
            }
            
            if (!Console_AddCommand(&perm_state->console, 
                                    String_Create("jump"), 
                                    CmdJump, 
                                    perm_state)) 
            {
                g_log("[Game] Failed to add 'jump' command\n");
                return false;
            }
            if (!Console_AddCommand(&perm_state->console, 
                                    String_Create("reset"), 
                                    CmdReset,
                                    perm_state)) {
                g_log("[Game] Failed to add 'reset' command\n");
                return false;
            }
        }
		perm_state->game_speed = 1.f;
		
#endif // INTERNAL
        
        //- NOTE(Momo): Sub arenas
        if (!Arena_Partition(&perm_state->mode_arena, 
                                 &perm_state->arena, 
                                 Mebibytes(1))) 
        {
            g_log("[Game] Failed to initialize mode arena\n");
            return false;
        }
        
        if (!Arena_Partition(&perm_state->asset_arena,
                                 &perm_state->arena,
                                 Mebibytes(16)))
        {
            g_log("[Game] Failed to initialize asset arena\n");
            return false;
        }
        
        if (!Arena_Partition(&perm_state->frame_arena,
                                 &perm_state->arena,
                                 Mebibytes(1)))
        {
            g_log("[Game] Failed to initialize frame arena\n");
            return false;
        }
		
        if (!Assets_Init(&perm_state->assets, 
						 &perm_state->asset_arena,
						 &perm_state->frame_arena)) {
            g_log("[Game] Failed to initialize assets\n");
            return false;
        }
		

#if INTERNAL		
		perm_state->asset_last_modified_time = g_platform->get_file_modified_time(GAME_ASSET_FILENAME);
#endif // INTERNAL

        perm_state->current_game_mode = Game_Mode_Type_None;
        perm_state->next_game_mode = Game_Mode_Type_Menu;
        perm_state->is_initialized = true;
		
        g_log("[Game] Init End\n");
        
    }
    g_mixer = &perm_state->mixer;
	g_scratch = &perm_state->frame_arena;
	
#if INTERNAL
	u64 current_asset_modified_time = g_platform->get_file_modified_time(GAME_ASSET_FILENAME);
	if (perm_state->asset_last_modified_time < current_asset_modified_time) {
		//- NOTE(Momo): Assets 
		g_log("[Game] Reloading game assets attempt\n");
		
		if (Assets_Init(&perm_state->assets, 
						&perm_state->asset_arena, 
						&perm_state->frame_arena)) 
		{
			g_log("[Game] Reloaded game assets\n");
			perm_state->asset_reload_retry_count = 0;
			perm_state->asset_last_modified_time = current_asset_modified_time;
		}
		else {
			g_log("[Game] Reloading game assets attempt #%d\n",
				  perm_state->asset_reload_retry_count);
			
			if(perm_state->asset_reload_retry_count++ > 256) {
				g_log("[Game] Game assets reload failed too many times\n");
				return false;
			}
			
		}
		
	}
#endif // INTERNAL
    g_assets = &perm_state->assets;
    
    
#if INTERNAL
    // NOTE(Momo): Input
    if (Platform_Input_IsButtonPoked(g_input->button_inspector)) {
        perm_state->inspector.is_active = !perm_state->inspector.is_active;
    }
    //START_PROFILING(Test);
	
    Inspector_Begin(&perm_state->inspector);
    Console_Update(&perm_state->console, dt);
    //END_PROFILING(Test);
    
    // NOTE(Momo): speed up/down
    if (Platform_Input_IsButtonPoked(g_input->button_speed_down)) {
        perm_state->game_speed -= 0.1f;
    }
    if (Platform_Input_IsButtonPoked(g_input->button_speed_up)) {
        perm_state->game_speed += 0.1f;
    }
	dt *= perm_state->game_speed;
	
#endif // INTERNAL
    
    
    // NOTE(Momo): Clean state/Switch states
    if (perm_state->next_game_mode != Game_Mode_Type_None) {
        Arena_Clear(&perm_state->mode_arena);
        Arena* mode_arena = &perm_state->mode_arena;
        
        switch(perm_state->current_game_mode) {
            case Game_Mode_Type_Main: {
                Main_Free(perm_state);
            } break;
            case Game_Mode_Type_Menu: {
                Menu_Free(perm_state);
            } break;
        }
        
        switch(perm_state->next_game_mode) {
            case Game_Mode_Type_Menu: {
                perm_state->main_mode = Arena_Push<Game_Mode_Main>(mode_arena);
                if(!Menu_Init(perm_state)) {
                    return false;
                }
                
            } break;
            case Game_Mode_Type_Main: {
                perm_state->main_mode = Arena_Push<Game_Mode_Main>(mode_arena); 
                if (!Main_Init(perm_state)){
                    return false;
                }
            } break;
			
            case Game_Mode_Type_Exit: {
                // Do nothing
            } break;
            default: {
                Assert(false);
            }
        }
        
        perm_state->current_game_mode = perm_state->next_game_mode;
        perm_state->next_game_mode = Game_Mode_Type_None;
    }
    
    //Draw_Prepare();
    Painter_Prepare(g_painter, render_commands, g_assets, C4F_GREY1);
    
#if INTERNAL
    if (perm_state->inspector.is_active){
        String buffer = {};
        
        buffer = String_Create("Mode Memory Left: ");
        Inspector_Push(&perm_state->inspector, buffer,
                       Arena_Remaining(&perm_state->mode_arena));
        buffer = String_Create("Mode Memory Used: ");
        Inspector_Push(&perm_state->inspector, buffer,
                       perm_state->mode_arena.used);
        
        buffer = String_Create("Asset Memory Left: ");
        Inspector_Push(&perm_state->inspector, buffer,
                       Arena_Remaining(&perm_state->asset_arena));
        buffer = String_Create("Asset Memory Used ");
        Inspector_Push(&perm_state->inspector, buffer,
                       perm_state->asset_arena.used);
        
        buffer = String_Create("Total Memory Left: ");
        Inspector_Push(&perm_state->inspector, buffer,
                       Arena_Remaining(&perm_state->arena));
        buffer = String_Create("Total Memory Used ");
        Inspector_Push(&perm_state->inspector, buffer,
                       perm_state->arena.used);
    }
#endif // INTERNAL
	
    // State update
    switch(perm_state->current_game_mode) {
        case Game_Mode_Type_Menu: {
            Menu_Update(perm_state,
                        dt);
        } break;
        case Game_Mode_Type_Main: {
            Main_Update(perm_state,
                        dt);
        } break; 
		
		
        case Game_Mode_Type_Exit: {
            // Do nothing
        } break;
        default: {
            Assert(false);
        }
    }
    
#if INTERNAL
    Console_Render(&perm_state->console);
    Inspector_End(&perm_state->inspector);
#endif // INTERNAL
    
    AudioMixer_Update(g_mixer, platform_audio);
    
    
    return perm_state->current_game_mode != Game_Mode_Type_Exit;
}