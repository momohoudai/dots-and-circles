/* date = September 8th 2021 0:07 pm */

static inline void
Main__ReadScore(Game_Mode_Main* mode) 
{
    u8 score_end_sym = 10;
    
    g_log("[Main] Reading score...\n");
    BigInt* high_score = &mode->high_score;
    Platform_File_Handle handle = g_platform->open_file(GAME_SAVE_FILENAME,
														Platform_File_Open_Mode_Read,
														Platform_File_Location_User);
    if (handle.error == 0) {
        defer { g_platform->close_file(&handle); };
        u8 byte;
        
        g_log("[Main] Read score: ");
        
        for (u32 i = 0; i < high_score->cap; ++i) {
            g_platform->read_file(&handle, i, sizeof(u8), &byte);
            if (byte != score_end_sym) {
                high_score->data[i] = byte;
                g_log("%d", byte);
            }
            else {
                high_score->count = i;
                break;
            }
            
        }
        g_log("\n");
        
        
    }
    else {
        // NOTE(Momo): File does not exist
        // TODO(Momo): Create a dummy file with 0 score
		g_log("[Main] Score file does not exist, so creating one\n");
        Platform_File_Handle write_handle = 
		g_platform->open_file(GAME_SAVE_FILENAME,
							  Platform_File_Open_Mode_Overwrite,
							  Platform_File_Location_User);
        if(write_handle.error == 0) {
            defer { g_platform->close_file(&write_handle); };
            
            u8 byte_to_write = 0;
            g_platform->write_file(&write_handle, 0, sizeof(u8), &byte_to_write);
            g_platform->write_file(&write_handle, 1, sizeof(u8), &score_end_sym);
        }
		else {
			g_log("[Main] Cannot create score file\n");
			
		}
    }
}


static inline void
Main__SaveScore(Game_Mode_Main* mode) 
{
    u8 score_end_sym = 10;
    
	if (mode->score <= mode->high_score) {
		return;
	}
	
    g_log("[Main] Saving score...\n");
	
    Platform_File_Handle write_handle = 
	g_platform->open_file(GAME_SAVE_FILENAME,
						  Platform_File_Open_Mode_Overwrite,
						  Platform_File_Location_User);
    if(write_handle.error == 0) {
        defer { g_platform->close_file(&write_handle); };
        u32 it = 0;
        g_log("[Main] Saved score: ");
        auto foreach = [](u8* value, Platform_File_Handle* write_handle, u32* it) {
            g_platform->write_file(write_handle, (*it), sizeof(u8), &(*value));
            g_log("%d", (*value));
            ++(*it);
        };
        BigInt_ForeachFromLeastSignificant(&mode->score, foreach, &write_handle, &it);
        g_log("\n");
        
        u8 byte_to_write = 0;
        g_platform->write_file(&write_handle, it, sizeof(u8), &score_end_sym);
        
    }
}
