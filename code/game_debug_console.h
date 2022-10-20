#ifndef __GAME_CONSOLE__
#define __GAME_CONSOLE__


typedef void (*Console_Callback)(struct Console* console, void* context, String args);

struct Console_Command {
    String key;
    Console_Callback callback;
    void* context;
};

struct Console_Line {
    StringBuffer text;
    c4f color;
};

struct Console {
    b8 is_active;
    
    Vec2f dims;
    Vec2f cur_pos;
    Vec2f start_pos;
    Vec2f end_pos;
    
    
    Array<Console_Line> info_lines;
    Console_Line input_line;
    
    // Backspace (to delete character) related
    // Maybe make an easing system?
    Timer start_pop_repeat_timer;
    Timer pop_repeat_timer;
    b8 is_start_pop;
    
    // Enter and Exit transitions for swag!
    Timer transit_timer;
    
    // List of commands
    List<Console_Command> commands;
};


static inline b8
Console_Init(Console* c, Arena* arena, u32 max_commands)
{
    const u32 line_length = 110;
    const u32 info_line_count = 5;
    
    c->transit_timer = Timer_Create(0.25f);
    
    c->dims = {GAME_DESIGN_WIDTH, 240.f};
    c->start_pos = {GAME_DESIGN_WIDTH/2, -c->dims.h/2};
    c->end_pos = {GAME_DESIGN_WIDTH/2,  c->dims.h/2};
    c->cur_pos = {};
    
    // NOTE(Momo): Timers related to type
    c->start_pop_repeat_timer = Timer_Create(0.5f);
    c->pop_repeat_timer = Timer_Create(0.1f); 
    
    if (!List_Alloc(&c->commands, arena, max_commands)) {
        return false;
    }
    
    if (!StringBuffer_Alloc(&c->input_line.text, arena, line_length)) {
        return false;
    }
    
    if (!Array_Alloc(&c->info_lines, arena, 5)) {
        return false;
    }
    for (u32 i = 0; i < c->info_lines.count; ++i) {
        if (!StringBuffer_Alloc(&c->info_lines[i].text, arena, line_length)){
            return false;
        }
    }
    
    return true;
}

static inline b8 
Console_AddCommand(Console* c, String key, 
                   Console_Callback callback, 
                   void* context)
{
    Console_Command* command = List_Push(&c->commands);
    if (command == nullptr) {
        return false;
    }
    command->key = key;
    command->callback = callback;
    command->context = context;
    
    return true;
}



static inline void
Console_PushInfo(Console* c, String str, c4f color) {
    for (u32 i = 0; i < c->info_lines.count - 1; ++i) {
        u32 j = c->info_lines.count - 1 - i;
        Console_Line* dest = c->info_lines + j;
        Console_Line* src = c->info_lines + j - 1;
        StringBuffer_Copy(&dest->text, src->text.str);
        dest->color = src->color;
    }
    c->info_lines[0].color = color;
    StringBuffer_Clear(&c->info_lines[0].text);
    StringBuffer_Copy(&c->info_lines[0].text, str);
}


static inline void 
Console_Pop(Console* c) {
    StringBuffer_Pop(&c->input_line.text);
}

static inline void 
Console_RemoveCommand(Console* c, String key) {
    for (u32 i = 0; i < c->commands.count; ++i) {
        if (c->commands[i].key == key) {
            List_Slear(&c->commands, i);
            return;
        }
    }
    
}

// Returns true if there is a new command
static inline void 
Console_Update(Console* c, f32 dt) 
{
    if (Platform_Input_IsButtonPoked(g_input->button_console)) {
        c->is_active = !c->is_active; 
    }
    
    // Transition
    {
        f32 p = EaseInQuad(Timer_Percent(&c->transit_timer));
        Vec2f delta = (c->end_pos - c->start_pos) * p; 
        c->cur_pos = c->start_pos + delta; 
    }
    
    if (c->is_active) {
        Timer_Tick(&c->transit_timer, dt);
        if (g_input->characters.count > 0 && 
            g_input->characters.count <= StringBuffer_Remaining(&c->input_line.text))
        {  
            StringBuffer_PushString(&c->input_line.text, g_input->characters.str);
        }
        
        // Remove character backspace logic
        if (Platform_Input_IsButtonDown(g_input->button_back)) {
            if(!c->is_start_pop) {
                Console_Pop(c);
                c->is_start_pop = true;
                Timer_SetToStart(&c->start_pop_repeat_timer);
                Timer_SetToStart(&c->pop_repeat_timer);
            }
            else {
                if (Timer_IsAtEnd(&c->start_pop_repeat_timer)) {
                    if(Timer_IsAtEnd(&c->pop_repeat_timer)) {
                        Console_Pop(c);
                        Timer_SetToStart(&c->pop_repeat_timer);
                    }
                    Timer_Tick(&c->pop_repeat_timer, dt);
                }
                Timer_Tick(&c->start_pop_repeat_timer, dt);
            }
        }
        else {
            c->is_start_pop = false; 
        }
        
        // Execute command
        String input_line_str = c->input_line.text.str;
        if (Platform_Input_IsButtonPoked(g_input->button_confirm)) {
            Console_PushInfo(c, input_line_str, C4F_WHITE);
            
            u32 min = 0;
            u32 max = input_line_str.count;
            for (u32 i = 0; i < c->input_line.text.count; ++i) {
                if (c->input_line.text.data[i] == ' ') {
                    max = i;
                    break;
                }
            }
            String command_str = String_Create(input_line_str, min, max);
            
            // Send a command to a callback
            for (u32 i = 0; i < c->commands.count; ++i) {
                Console_Command* command = c->commands + i;
                if (command->key == command_str) {
                    command->callback(c, 
                                      command->context, 
                                      input_line_str);
                }
            }
            
            StringBuffer_Clear(&c->input_line.text);
            
        }
    }
    else {
        Timer_Untick(&c->transit_timer, dt);
    }
    
    
}


static inline void
Console_Render(Console* c) 
{
    if (Timer_IsAtStart(&c->transit_timer)) {
        return;
    }
    
    Font* font= Assets_GetFont(g_assets, Font_ID_Debug);
    f32 bottom = c->cur_pos.y - c->dims.h * 0.5f;
    f32 left = c->cur_pos.x - c->dims.w * 0.5f;
    f32 line_height = c->dims.h / (c->info_lines.count + 1);
    f32 font_size = line_height * 0.9f;
    f32 font_height = Font_Height(font) * font_size;
    
    f32 padding_h = (line_height - font_height) * 0.5f  + Abs(font->descent) * font_size; 
    f32 padding_w = c->dims.w * 0.005f;
    
    Painter_DrawQuad(g_painter, c->cur_pos, c->dims, C4F_GREY3);
    
    // Input bar
    {
        Painter_DrawQuad(g_painter, 
                         Vec2f{ c->cur_pos.x, bottom + line_height * 0.5f }, 
                         Vec2f{ c->dims.w, line_height } , 
                         C4F_GREY2);
        
        
    }
    
    // Draw info text
    {
        for (u32 i = 0; i < c->info_lines.count ; ++i) {
            Vec2f text_pos = {};
            text_pos.x = left + padding_w;
            text_pos.y = bottom + ((i+1) * line_height) + padding_h;
            
            String infoLineCStr = c->info_lines[i].text.str;
            Painter_DrawTextLeft(g_painter,
                                 Font_ID_Debug, 
                                 text_pos,
                                 c->info_lines[i].text.str,
                                 font_size,
                                 c->info_lines[i].color);
        }
        
    }
    // NOTE(Momo): Draw input text
    {
        Vec2f text_position = {};
        text_position.x = left + padding_w;
        text_position.y = bottom + padding_h;
        
        Painter_DrawTextLeft(g_painter,
                             Font_ID_Debug, 
                             text_position,
                             c->input_line.text.str,
                             font_size,
                             C4F_WHITE);
    }
}





#endif
