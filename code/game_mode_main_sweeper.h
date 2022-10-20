/* date = October 19th 2021 1:45 pm */

#ifndef GAME_MODE_MAIN_SWEEPER_H
#define GAME_MODE_MAIN_SWEEPER_H

struct Sweeper {
	Line2f top_line;
	Line2f btm_line;
	b8 is_active;
	f32 thickness;
};

#endif //GAME_MODE_MAIN_SWEEPER_H
