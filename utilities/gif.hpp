#define USE_GIF_SIZES

#define GIF_FRAME_MAX (120 * 64 * 4)
#define SCRXSIZE 96
#define SCRYSIZE 64

#define GIF_IDLE 0
#define GIF_START 1
#define GIF_FRAME 2
#define GIF_END 3

#ifdef WINVER
#include "gui.h"
#else
#include "../core/coretypes.hpp"
#endif

extern int gif_write_state;
extern int gif_file_size;
extern char gif_file_name[256];
extern bool gif_autosave;
extern bool gif_use_increasing;
extern uint16_t gif_base_delay;
extern int gif_xs;
extern int gif_indiv_xs;
extern int gif_ys;
extern uint8_t gif_frame[GIF_FRAME_MAX];
extern int gif_time;
extern int gif_newframe;
extern int gif_colors;
extern int gif_base_delay_start;
extern int gif_file_num;
extern uint32_t gif_size;
extern bool gif_bw;

void gif_writer(int shades);
