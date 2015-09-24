#ifndef _CALC_H
#define _CALC_H

#include "../core/coretypes.hpp"

#include "../core/core.hpp"
#include "../hardware/lcd.hpp"
#include "../hardware/keys.hpp"
#include "../hardware/link.hpp"

#include "../utilities/label.hpp"

typedef enum {
	GDS_IDLE,
	GDS_STARTING,
	GDS_RECORDING,
	GDS_ENDING
} gif_disp_states;

#define MIN_BLOCK_SIZE 16
#define MAX_FLASH_PAGE_SIZE 0x80
#define MAX_RAM_PAGE_SIZE 0x08
typedef struct profiler {
	bool running;
	int blockSize;
	long long totalTime;
	long flash_data[MAX_FLASH_PAGE_SIZE][PAGE_SIZE / MIN_BLOCK_SIZE];
	long ram_data[MAX_RAM_PAGE_SIZE][PAGE_SIZE / MIN_BLOCK_SIZE];
} profiler_t;

#define KEY_STRING_SIZE 56
struct key_string {
	char *text;
	int group;
	int bit;
	int repeat;
	struct key_string *next;
};

typedef struct tagCALC {
	void (*breakpoint_callback)(struct tagCALC *);
	int slot;
#define PATH_MAX 4097
	char rom_path[PATH_MAX];
	char rom_version[32];
	int model;

	time_t time_error;

	bool active;
	CPU_t cpu;
	memory_context_t mem_c;
	timer_context_t timer_c;

	bool running;
	bool auto_turn_on;
	int speed;
	uint8_t breakpoints[0x10000];
	label_struct labels[6000];
	profiler_t profiler;

	char labelfn[256];
	applist_t applist;
	apphdr_t *last_transferred_app;

	gif_disp_states gif_disp_state;

} calc_t;

#ifdef WITH_BACKUPS
typedef struct DEBUG_STATE {
	SAVESTATE_t *save;
	struct DEBUG_STATE *next, *prev;
} debugger_backup;
#endif

#define MAX_CALCS	8
#define MAX_SPEED 100*50

typedef struct tagCALC CALC, *LPCALC;

void calc_turn_on(LPCALC);
LPCALC calc_slot_new(void);
uint32_t calc_count(void);
int calc_reset(LPCALC);
int CPU_reset(CPU_t *);
int calc_run_frame(LPCALC);
int calc_run_seconds(LPCALC, double);
int calc_run_timed(LPCALC, time_t);
int calc_run_all(void);
bool calc_start_screenshot(calc_t *calc, const char *filename);
void calc_stop_screenshot(calc_t *calc);

#ifdef WITH_BACKUPS
void do_backup(LPCALC);
void restore_backup(int index, LPCALC);
void init_backups();
void free_backups(LPCALC);
void free_backup(debugger_backup *);
#endif

bool rom_load(LPCALC lpCalc, const char * FileName);
void calc_slot_free(LPCALC);

void calc_unpause_linked();
void calc_pause_linked();

int calc_init_83p(LPCALC);
int calc_init_84p(LPCALC);
int calc_init_83pse(LPCALC);
LPCALC calc_from_cpu(CPU_t *);
LPCALC calc_from_memc(memc *);
void calc_erase_certificate(unsigned char *, int);
void port_debug_callback(void *, void *);
void mem_debug_callback(void *);

#ifdef CALC_C
#define GLOBAL
#else
#define GLOBAL extern
#endif

GLOBAL calc_t calcs[MAX_CALCS];
//GLOBAL LPCALC lpDebuggerCalc;

#ifdef WITH_BACKUPS
#define MAX_BACKUPS 10
GLOBAL debugger_backup * backups[MAX_CALCS];
GLOBAL int number_backup;
GLOBAL int current_backup_index;
GLOBAL int num_backup_per_sec;
#endif

#ifdef WITH_AVI
#include "avi_utils.h"
#include "avifile.h"
GLOBAL CAviFile *currentAvi;
GLOBAL HAVI recording_avi;
GLOBAL bool is_recording;
#endif

GLOBAL uint32_t frame_counter;
GLOBAL int startX;
GLOBAL int startY;
GLOBAL bool exit_save_state;
GLOBAL bool check_updates;
GLOBAL bool new_calc_on_load_files;
GLOBAL bool do_backups;
GLOBAL bool break_on_exe_violation;
GLOBAL bool break_on_invalid_flash;
GLOBAL bool sync_cores;
GLOBAL link_t *link_hub[MAX_CALCS + 1];
GLOBAL int link_hub_count;
GLOBAL int calc_waiting_link;

GLOBAL const char *CalcModelTxt[]
#ifdef CALC_C
= {	//"???",
	("TI-81"),
	("TI-82"),
	("TI-83"),
	("TI-85"),
	("TI-86"),
	("TI-73"),
	("TI-83+"),
	("TI-83+SE"),
	("TI-84+"),
	("TI-84+SE"),
	("???")}
#endif
;

#define _HAS_CALC_H
#endif
