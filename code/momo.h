#ifndef __MOMO_H__
#define __MOMO_H__

#include <math.h>
#include <cstdint>
#include <cstdarg>

// Basic types - Don't really need their own namespaces
#include "momo_base.h"
#include "momo_maths.h"
#include "momo_colors.h"
#include "momo_bitwise.h"
#include "momo_easing.h"

// The ones below need their own namespace prefixes
#include "momo_crc.h"
#include "momo_bonk.h"
#include "momo_arena.h"
#include "momo_timer.h"
#include "momo_sort.h"
#include "momo_rect_packer.h"
#include "momo_string.h"
#include "momo_random.h"
#include "momo_stream.h"
#include "momo_mailbox.h"
#include "momo_array.h"
#include "momo_list.h"
#include "momo_linked_list.h"
#include "momo_queue.h"
#include "momo_bigint.h"

// File formats
#include "momo_wav.h"
#include "momo_png.h"
#include "momo_png.cpp"

#endif // __MOMO_H__