#ifndef __MM_RECT_PACKER__
#define __MM_RECT_PACKER__

enum RectPacker_SortType {
    RectPacker_SortType_Width,
    RectPacker_SortType_Height,
    RectPacker_SortType_Area,
    RectPacker_SortType_Perimeter,
    RectPacker_SortType_BiggerSide,
    RectPacker_SortType_Pathological,
};

static inline void
_SortRectForPacker(Rect2u* rects,
                   SortEntry* sort_entries,
                   u32 sort_entry_count,
                   RectPacker_SortType sort_type)
{
    switch (sort_type) {
        case RectPacker_SortType_Width: {
            for (u32 i = 0; i < sort_entry_count; ++i) {
                u32 rect_w = Rect_Width(rects[i]);
                f32 key = -(f32)rect_w;
                sort_entries[i].key = key;
                sort_entries[i].index = i;
            }
        } break;
        case RectPacker_SortType_Height: {
            for (u32 i = 0; i < sort_entry_count; ++i) {
                u32 rect_h = Rect_Height(rects[i]);
                f32 key = -(f32)rect_h;
                sort_entries[i].key = key;
                sort_entries[i].index = i;
            }
        } break;
        case RectPacker_SortType_Area: {
            for (u32 i = 0; i < sort_entry_count; ++i) {
                u32 rect_w = Rect_Width(rects[i]);
                u32 rect_h = Rect_Height(rects[i]);
                f32 key = -(f32)(rect_w * rect_h);
                sort_entries[i].key = key;
                sort_entries[i].index = i;
            }
        } break;
        case RectPacker_SortType_Perimeter: {
            for (u32 i = 0; i < sort_entry_count; ++i) {
                u32 rect_w = Rect_Width(rects[i]);
                u32 rect_h = Rect_Height(rects[i]);
                f32 key = -(f32)(rect_w + rect_h);
                sort_entries[i].key = key;
                sort_entries[i].index = i;
            }
        } break;
        case RectPacker_SortType_BiggerSide: {
            for (u32 i = 0; i < sort_entry_count; ++i) {
                u32 rect_w = Rect_Width(rects[i]);
                u32 rect_h = Rect_Height(rects[i]);
                f32 key = -(f32)(Max(rect_w, rect_h));
                sort_entries[i].key = key;
                sort_entries[i].index = i;
            }
        } break;
        case RectPacker_SortType_Pathological: {
            for (u32 i = 0; i < sort_entry_count; ++i) {
                u32 rect_w = Rect_Width(rects[i]);
                u32 rect_h = Rect_Height(rects[i]);
                
                u32 wh_max = Max(rect_w, rect_h);
                u32 wh_min = Min(rect_w, rect_h);
                f32 key = -(f32)(wh_max/wh_min * rect_w * rect_h);
                sort_entries[i].key = key;
                sort_entries[i].index = i;
            }
        } break;
        
    }
    
    QuickSort(sort_entries, sort_entry_count);
    
}

// NOTE(Momo): rects WILL be sorted after this function
static inline b8
PackRects(Arena* arena,
          u32 total_width,
          u32 total_height,
          Rect2u* rects, 
          u32 rect_count, 
          u32 padding,
          RectPacker_SortType sort_type) 
{
    Arena_Marker scratch = Arena_Mark(arena);
    defer { Arena_Revert(&scratch); };
    auto* sort_entries = Arena_PushArray<SortEntry>(arena, rect_count);
    
    _SortRectForPacker(rects, sort_entries, rect_count, sort_type);
    
    
    u32 current_node_count = 0;
    
    auto* nodes = Arena_PushArray<Rect2u>(arena, rect_count+1);
    nodes[current_node_count++] = Rect_CreateDims(Vec_Create(total_width, total_height));
    
    for (u32 i = 0; i < rect_count; ++i) {
        Rect2u* rect = rects + sort_entries[i].index;
        
        // NOTE(Momo): padding*2 because there are 2 sides
        u32 rect_width = Rect_Width(*rect) + padding*2;
        u32 rect_height = Rect_Height(*rect) + padding*2;
        
        // NOTE(Momo): Iterate the empty spaces backwards to find the best fit index
        u32 chosen_space_index = current_node_count;
        for (u32  j = 0; j < chosen_space_index ; ++j ) {
            u32 index = chosen_space_index - j - 1;
            Rect2u space = nodes[index];
            u32 space_w = Rect_Width(space);
            u32 space_h = Rect_Height(space);
            
            // NOTE(Momo): Check if the image fits
            if (rect_width <= space_w && rect_height <= space_h) {
                chosen_space_index = index;
                break;
            }
        }
        
        
        // NOTE(Momo): If an empty space that can fit is found, 
        // we remove that space and split.
        if (chosen_space_index == current_node_count) {
            return false;
        }
        
        // NOTE(Momo): Swap and pop the chosen space
        Rect2u chosen_space = nodes[chosen_space_index];
        u32 chosen_space_w = Rect_Width(chosen_space);
        u32 chosen_space_h = Rect_Height(chosen_space);
        
        if (current_node_count > 0) {
            nodes[chosen_space_index] = nodes[current_node_count-1];
            --current_node_count;
        }
        
        // NOTE(Momo): Split if not perfect fit
        if (chosen_space_w != rect_width && chosen_space_h == rect_height) {
            // Split right
            Rect2u split_space_right = 
                Rect_CreatePosDims(Vec_Create(chosen_space.min.x + rect_width, chosen_space.min.y),
                                   Vec_Create(chosen_space_w - rect_width, chosen_space_h ));
            nodes[current_node_count++] = split_space_right;
        }
        else if (chosen_space_w == rect_width && chosen_space_h != rect_height) {
            // Split down
            Rect2u split_space_down = Rect_CreatePosDims(Vec_Create(chosen_space.min.x, chosen_space.min.y + rect_height),
                                                         Vec_Create( chosen_space_w, chosen_space_h - rect_height));
            nodes[current_node_count++] = split_space_down;
        }
        else if (chosen_space_w != rect_width && chosen_space_h != rect_height) {
            // Split right
            Rect2u split_space_right = 
                Rect_CreatePosDims(Vec_Create(chosen_space.min.x + rect_width, chosen_space.min.y),
                                   Vec_Create( chosen_space_w - rect_width, rect_height));
            
            // Split down
            Rect2u split_space_down = 
                Rect_CreatePosDims(Vec_Create(chosen_space.min.x, chosen_space.min.y + rect_height),
                                   Vec_Create(chosen_space_w, chosen_space_h - rect_height));
            
            // Choose to insert the bigger one first before the smaller one
            u32 split_space_right_w = Rect_Width(split_space_right);
            u32 split_splace_right_h = Rect_Height(split_space_right);
            u32 split_space_down_w = Rect_Width(split_space_down);
            u32 split_space_down_h = Rect_Height(split_space_down);
            
            u32 right_area = split_space_right_w * split_splace_right_h;
            u32 down_area = split_space_down_w * split_space_down_h;
            
            if (right_area > down_area) {
                nodes[current_node_count++] = split_space_right;
                nodes[current_node_count++] = split_space_down;
            }
            else {
                nodes[current_node_count++] = split_space_down;
                nodes[current_node_count++] = split_space_right;
            }
            
        }
        
        // NOTE(Momo): Translate the rect
        (*rect) = Rect_Translate((*rect),
                                 chosen_space.min + Vec_Create(padding, padding));
    }
    
    return true;
}


#endif
