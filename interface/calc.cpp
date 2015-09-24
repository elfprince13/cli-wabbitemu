#define CALC_C
#include "calc.h"
#include "core.h"
#include "81hw.h"
#include "83hw.h"
#include "83phw.h"
#include "83psehw.h"
#include "86hw.h"
#include "device.h"
#include "var.h"
#include "gif.h"
#include "gifhandle.h"
#include "link.h"
#include "keys.h"

#ifdef _WINDOWS
#include "disassemble.h"
#include "CCalcAddress.h"
#include "CPage.h"
#include "exportvar.h"
#include "guiwizard.h"
#include "guibuttons.h"
#endif

/*
 * Determine the slot for a new calculator.  Return a pointer to the calc
 */
calc_t *calc_slot_new(void) {
#ifdef WITH_BACKUPS
	current_backup_index = 10;
#endif
	if (link_hub[MAX_CALCS] == nullptr) {
		memset(link_hub, 0, sizeof(link_hub));
		link_t *hub_link = (link_t *) malloc(sizeof(link_t)); 
		if (!hub_link) {
			printf("Couldn't allocate memory for link hub\n");
		}
		hub_link->host		= 0;				//neither lines set
		hub_link->client	= &hub_link->host;	//nothing plugged in.
		link_hub[MAX_CALCS] = hub_link;
	}
	int i;
	for (i = 0; i < MAX_CALCS; i++) {
		if (calcs[i].active == false) {
			memset(&calcs[i], 0, sizeof(calc_t));
			calcs[i].active = true;
			calcs[i].gif_disp_state = GDS_IDLE;
			calcs[i].speed = 100;
			calcs[i].slot = i;
			return &calcs[i];
		}
	}
	return nullptr;
}

uint32_t calc_count(void) {
	uint32_t count = 0;

	int i;
	for (i = 0; i < MAX_CALCS; i++) {
		if (calcs[i].active == true)
			count++;
	}
	return count;
}

/* 81 */
int calc_init_81(CALC* lpCalc, char *version) {
	/* INTIALIZE 81 */
	//v2 is basically an 82
	if (version[0] == '2') {
		memory_init_81(&lpCalc->mem_c);
		tc_init(&lpCalc->timer_c, MHZ_2);
		CPU_init(&lpCalc->cpu, &lpCalc->mem_c, &lpCalc->timer_c);
		ClearDevices(&lpCalc->cpu);
		device_init_83(&lpCalc->cpu, 1);
	} else {
		memory_init_81(&lpCalc->mem_c);
		tc_init(&lpCalc->timer_c, MHZ_2);
		CPU_init(&lpCalc->cpu, &lpCalc->mem_c, &lpCalc->timer_c);
		ClearDevices(&lpCalc->cpu);
		device_init_81(&lpCalc->cpu);
	
	}
	/* END INTIALIZE 81 */

#ifdef WINVER // FIXME: dirty cheater!
	lpCalc->flash_cond_break = (LPBREAKPOINT *) calloc(lpCalc->mem_c.flash_size, sizeof(LPBREAKPOINT *));
	lpCalc->ram_cond_break = (LPBREAKPOINT *) calloc(lpCalc->mem_c.ram_size, sizeof(LPBREAKPOINT *));
	if (version[0] == '2') {
		lpCalc->audio			= &lpCalc->cpu.pio.link->audio;
		lpCalc->audio->enabled	= false;
		lpCalc->audio->init		= false;
		lpCalc->audio->timer_c	= &lpCalc->timer_c;
	}
#endif
	return true;
}

/*  82 83 */
static bool calc_init_83(CALC* lpCalc, char *os) {
	/* INTIALIZE 83 */
	memory_init_83(&lpCalc->mem_c);
	tc_init(&lpCalc->timer_c, MHZ_6);
	CPU_init(&lpCalc->cpu, &lpCalc->mem_c, &lpCalc->timer_c);
	ClearDevices(&lpCalc->cpu);
	if (lpCalc->model == TI_82) {
		if (memcmp(os, "19.006", 6)==0) {
			device_init_83(&lpCalc->cpu, 0);
		} else {
			device_init_83(&lpCalc->cpu, 1);
		}
	} else {
		device_init_83(&lpCalc->cpu, 0);
	}
	/* END INTIALIZE 83 */

#ifdef WINVER // FIXME: dirty cheater!
	lpCalc->flash_cond_break = (LPBREAKPOINT *) calloc(lpCalc->mem_c.flash_size, sizeof(LPBREAKPOINT *));
	lpCalc->ram_cond_break = (LPBREAKPOINT *) calloc(lpCalc->mem_c.ram_size, sizeof(LPBREAKPOINT *));
	lpCalc->audio			= &lpCalc->cpu.pio.link->audio;
	lpCalc->audio->enabled	= false;
	lpCalc->audio->init		= false;
	lpCalc->audio->timer_c	= &lpCalc->timer_c;
#endif
	return true;
}

/* 85 86 */
static int calc_init_86(CALC* lpCalc) {

	/* INTIALIZE 86 */
	memory_init_86(&lpCalc->mem_c);
	tc_init(&lpCalc->timer_c, MHZ_4_8);
	CPU_init(&lpCalc->cpu, &lpCalc->mem_c, &lpCalc->timer_c);
	ClearDevices(&lpCalc->cpu);
	device_init_86(&lpCalc->cpu);
	/* END INTIALIZE 86 */

#ifdef WINVER // FIXME: dirty cheater!
	lpCalc->flash_cond_break = (LPBREAKPOINT *) calloc(lpCalc->mem_c.flash_size, sizeof(LPBREAKPOINT *));
	lpCalc->ram_cond_break = (LPBREAKPOINT *) calloc(lpCalc->mem_c.ram_size, sizeof(LPBREAKPOINT *));
	lpCalc->audio			= &lpCalc->cpu.pio.link->audio;
	lpCalc->audio->enabled	= false;
	lpCalc->audio->init		= false;
	lpCalc->audio->timer_c	= &lpCalc->timer_c;
#endif
	return 0;
}

/* 73 83+ */
int calc_init_83p(CALC* lpCalc) {
	/* INTIALIZE 83+ */
	memory_init_83p(&lpCalc->mem_c);
	tc_init(&lpCalc->timer_c, MHZ_6);
	CPU_init(&lpCalc->cpu, &lpCalc->mem_c, &lpCalc->timer_c);
	ClearDevices(&lpCalc->cpu);
	device_init_83p(&lpCalc->cpu);
	/* END INTIALIZE 83+ */

#ifdef WINVER // FIXME: dirty cheater!
	lpCalc->flash_cond_break = (LPBREAKPOINT *) calloc(lpCalc->mem_c.flash_size, sizeof(LPBREAKPOINT *));
	lpCalc->ram_cond_break = (LPBREAKPOINT *) calloc(lpCalc->mem_c.ram_size, sizeof(LPBREAKPOINT *));
	lpCalc->audio			= &lpCalc->cpu.pio.link->audio;
	lpCalc->audio->enabled	= false;
	lpCalc->audio->init		= false;
	lpCalc->audio->timer_c	= &lpCalc->timer_c;
#endif
	return 0;
}

/* 83+se 84+se */
int calc_init_83pse(CALC* lpCalc) {
	/* INTIALIZE 83+se */
	memory_init_83pse(&lpCalc->mem_c);
	tc_init(&lpCalc->timer_c, MHZ_6);
	CPU_init(&lpCalc->cpu, &lpCalc->mem_c, &lpCalc->timer_c);
	ClearDevices(&lpCalc->cpu);
	device_init_83pse(&lpCalc->cpu);
	/* END INTIALIZE 83+se */
#ifdef WINVER // FIXME: dirty cheater!
	lpCalc->flash_cond_break = (LPBREAKPOINT *) calloc(lpCalc->mem_c.flash_size, sizeof(LPBREAKPOINT *));
	lpCalc->ram_cond_break = (LPBREAKPOINT *) calloc(lpCalc->mem_c.ram_size, sizeof(LPBREAKPOINT *));
	lpCalc->audio			= &lpCalc->cpu.pio.link->audio;
	lpCalc->audio->enabled	= false;
	lpCalc->audio->init		= false;
	lpCalc->audio->timer_c	= &lpCalc->timer_c;
#endif
	return 0;
}

/* 84+ */
int calc_init_84p(CALC* lpCalc) {
	/* INTIALIZE 84+ */
	memory_init_84p(&lpCalc->mem_c);
	tc_init(&lpCalc->timer_c, MHZ_6);
	CPU_init(&lpCalc->cpu, &lpCalc->mem_c, &lpCalc->timer_c);
	ClearDevices(&lpCalc->cpu);
	device_init_83pse(&lpCalc->cpu);
#ifdef WITH_BACKUPS
	init_backups();
#endif
	/* END INTIALIZE 84+ */

#ifdef WINVER // FIXME: dirty cheater!
	lpCalc->flash_cond_break = (breakpoint_t **) calloc(lpCalc->mem_c.flash_pages, PAGE_SIZE);
	lpCalc->ram_cond_break = (breakpoint_t **) calloc(lpCalc->mem_c.ram_pages, PAGE_SIZE);
	lpCalc->audio			= &lpCalc->cpu.pio.link->audio;
	lpCalc->audio->enabled	= false;
	lpCalc->audio->init		= false;
	lpCalc->audio->timer_c	= &lpCalc->timer_c;
#endif
	return 0;
}


void calc_erase_certificate(unsigned char *mem, int size) {
	if (mem == nullptr || size < 32768) return;

	memset(mem + size - 32768, 0xFF, PAGE_SIZE);

	mem[size - 32768]				= 0x00;
	mem[size - 32768 + 0x1FE0]		= 0x00;
	mem[size - 32768 + 0x1FE1]		= 0x00;
	return;
}

#define BOOTFREE_VER "11.246"
void check_bootfree_and_update(CALC* lpCalc) {
	uint8_t *bootFreeString = lpCalc->mem_c.flash + (lpCalc->mem_c.flash_pages - 1) * PAGE_SIZE + 0x0F;
	if (*bootFreeString != '1') {
		//not using bootfree
		return;
	}
	if (bootFreeString[1] == '.') {
		//using normal bootpage
		return;
	}
	if (!strcmp((const char *) bootFreeString, BOOTFREE_VER)) {
		return;
	}
#ifdef WINVER
	char hexFile[PATH_MAX];
	ExtractBootFree(lpCalc->model, hexFile);
	FILE *file;
	fopen(&file, hexFile, ("rb"));
	writeboot(file, &lpCalc->mem_c, -1);
	fclose(file);
	fopen(&file, lpCalc->rom_path, ("wb"));
	if (file) {
		fclose(file);
		MFILE *mfile = ExportRom(lpCalc->rom_path, lpCalc);
		mclose(mfile);
	}
#endif
}

bool rom_load(CALC* lpCalc, const char * FileName) {
	if (lpCalc == nullptr) {
		return false;
	}
	TIFILE_t* tifile = newimportvar(FileName, false);
	if (tifile == nullptr) {
		return false;
	}

	lpCalc->speed = 100;
	if (lpCalc->active) {
		calc_slot_free(lpCalc);
	}
	lpCalc->model = tifile->model;

	if (tifile->type == SAV_TYPE) {
		lpCalc->active 	= true;
		switch (tifile->model) {
			case TI_82:
			case TI_83: {
				int size;
				char *rom = GetRomOnly(tifile->save, &size);
				char VerString[64];
				FindRomVersion(	tifile->model, VerString, (unsigned char *) rom, size);
				calc_init_83(lpCalc, VerString);
				break;
			}
			case TI_73:
			case TI_83P:
				calc_init_83p(lpCalc);
				break;
			case TI_84PSE:
			case TI_83PSE:
				calc_init_83pse(lpCalc);
				break;
			case TI_84P:
				calc_init_84p(lpCalc);
				break;
			case TI_85:
			case TI_86:
				calc_init_86(lpCalc);
				break;
			default:
				FreeTiFile(tifile);
				return false;
		}

		LoadSlot(tifile->save, lpCalc);
#ifdef WINVER
		StringCbCopy(lpCalc->rom_path, sizeof(lpCalc->rom_path), FileName);
#else
		_tcscpy_s(lpCalc->rom_path, FileName);
#endif
		FindRomVersion(tifile->model, lpCalc->rom_version, lpCalc->mem_c.flash, lpCalc->mem_c.flash_size);
	} else if (tifile->type == ROM_TYPE) {

		switch (tifile->model) {
			case TI_81:
				calc_init_81(lpCalc, tifile->rom->version);
				memcpy(	lpCalc->cpu.mem_c->flash,
						tifile->rom->data,
						(lpCalc->cpu.mem_c->flash_size<=tifile->rom->size)?lpCalc->cpu.mem_c->flash_size:tifile->rom->size);
				break;
			case TI_82:
			case TI_83:
				calc_init_83(lpCalc, tifile->rom->version);
				memcpy(	lpCalc->cpu.mem_c->flash,
						tifile->rom->data,
						(lpCalc->cpu.mem_c->flash_size<=tifile->rom->size)?lpCalc->cpu.mem_c->flash_size:tifile->rom->size);
				break;
			case TI_85:
			case TI_86:
				calc_init_86(lpCalc);
				memcpy(	lpCalc->cpu.mem_c->flash,
						tifile->rom->data,
						(lpCalc->cpu.mem_c->flash_size<=tifile->rom->size)?lpCalc->cpu.mem_c->flash_size:tifile->rom->size);
				break;
			case TI_73:
			case TI_83P:
				calc_init_83p(lpCalc);
				memcpy(	lpCalc->cpu.mem_c->flash,
						tifile->rom->data,
						(lpCalc->cpu.mem_c->flash_size<=tifile->rom->size)?lpCalc->cpu.mem_c->flash_size:tifile->rom->size);
				calc_erase_certificate(lpCalc->cpu.mem_c->flash,lpCalc->cpu.mem_c->flash_size);
				break;
			case TI_84P:
				calc_init_84p(lpCalc);
				memcpy(	lpCalc->cpu.mem_c->flash,
						tifile->rom->data,
						(lpCalc->cpu.mem_c->flash_size<=tifile->rom->size)?lpCalc->cpu.mem_c->flash_size:tifile->rom->size);
				calc_erase_certificate(lpCalc->cpu.mem_c->flash,lpCalc->cpu.mem_c->flash_size);
				break;
			case TI_84PSE:
			case TI_83PSE:
				calc_init_83pse(lpCalc);
				memcpy(	lpCalc->cpu.mem_c->flash,
						tifile->rom->data,
						(lpCalc->cpu.mem_c->flash_size<=tifile->rom->size)?lpCalc->cpu.mem_c->flash_size:tifile->rom->size);
				calc_erase_certificate(lpCalc->cpu.mem_c->flash,lpCalc->cpu.mem_c->flash_size);
				break;
			default:
				FreeTiFile(tifile);
				return false;
		}

		lpCalc->active = true;
		memcpy(lpCalc->rom_version, tifile->rom->version, sizeof(lpCalc->rom_version));
#ifdef WINVER
		StringCbCopy(lpCalc->rom_path, sizeof(lpCalc->rom_path), FileName);
#else
		_tcscpy_s(lpCalc->rom_path, FileName);
#endif
		

	} else {
		lpCalc = nullptr;
		return false;
	}
	if (lpCalc != nullptr) {
		lpCalc->cpu.pio.model = lpCalc->model;
#ifdef WINVER
extern keyprog_t keygrps[256];
extern keyprog_t defaultkeys[256];
extern keyprog_t keysti86[256];
		if (lpCalc->model == TI_86 || lpCalc->model == TI_85) {
			memcpy(keygrps, keysti86, sizeof(keyprog_t) * 256);
		} else {
			memcpy(keygrps, defaultkeys, sizeof(keyprog_t) * 256);
		}
#endif
		if (lpCalc->model >= TI_73) {
			check_bootfree_and_update(lpCalc);
		}
		if (tifile->save == nullptr) {
			calc_reset(lpCalc);
			if (lpCalc->auto_turn_on) {
				calc_turn_on(lpCalc);
			}
		}
	}

	FreeTiFile(tifile);
	return true;
}

void calc_slot_free(CALC* lpCalc) {
	if (lpCalc == nullptr)
		return;

	if (lpCalc->active) {
		lpCalc->active = false;

#ifdef WINVER
		KillSound(lpCalc->audio);
		lpCalc->audio = nullptr;

		free(lpCalc->flash_cond_break);
		lpCalc->flash_cond_break = nullptr;
		free(lpCalc->ram_cond_break);
		lpCalc->ram_cond_break = nullptr;

		FreeKeypressHistory(lpCalc);
#endif

		free(lpCalc->mem_c.flash);
		lpCalc->mem_c.flash = nullptr;
		free(lpCalc->mem_c.ram);
		lpCalc->mem_c.ram = nullptr;
		free(lpCalc->mem_c.flash_break);
		lpCalc->mem_c.flash_break = nullptr;
		free(lpCalc->mem_c.ram_break);
		lpCalc->mem_c.ram_break = nullptr;

		if (lpCalc->slot > 0 && link_connected_hub(lpCalc->slot)) {
			link_hub[lpCalc->slot] = nullptr;
		}
		free(lpCalc->cpu.pio.link);
		lpCalc->cpu.pio.link = nullptr;

		free(lpCalc->cpu.pio.keypad);
		lpCalc->cpu.pio.keypad = nullptr;

		free(lpCalc->cpu.pio.stdint);
		lpCalc->cpu.pio.stdint = nullptr;

		free(lpCalc->cpu.pio.se_aux);
		lpCalc->cpu.pio.se_aux = nullptr;

		free(lpCalc->cpu.pio.lcd);
		lpCalc->cpu.pio.lcd = nullptr;

#ifdef WITH_BACKUPS
		if (do_backups)
			free_backups(lpCalc);
#endif
	}
}

void calc_turn_on(CALC* lpCalc)
{
	bool running = lpCalc->running;
	lpCalc->running = true;
	calc_run_seconds(lpCalc, 2.0);
	keypad_press(&lpCalc->cpu, KEYGROUP_ON, KEYBIT_ON);
	calc_run_seconds(lpCalc, 1.0);
	keypad_release(&lpCalc->cpu, KEYGROUP_ON, KEYBIT_ON);
#ifdef QUICKLOOK
	calc_run_seconds(lpCalc, 0.5);
#endif
	lpCalc->running = running;
}

int calc_reset(CALC* lpCalc) {
	CPU_reset(&lpCalc->cpu);
	LCD_timer_refresh(&lpCalc->cpu);
	//calc_turn_on(lpCalc);
	return 0;
}

/* Clear RAM and start calculator at $0000 
 * 10/20/10 Calc84 says that starting at $0000 is wrong we need to start on the boot page,
 * 6/29/11 which turns out to be in bank 0 so we start at $0000 anyway.
 */
int CPU_reset(CPU_t *lpCPU) {
	lpCPU->sp			= 0;
	lpCPU->interrupt	= false;
	lpCPU->imode		= 1;
	lpCPU->ei_block		= false;
	lpCPU->iff1			= false;
	lpCPU->iff2			= false;
	lpCPU->halt			= false;
	lpCPU->read			= false;
	lpCPU->write		= false;
	lpCPU->output		= false;
	lpCPU->input		= false;
	lpCPU->prefix		= 0;
	lpCPU->pc			= 0;
	lpCPU->mem_c->port27_remap_count = 0;
	lpCPU->mem_c->port28_remap_count = 0;
	lpCPU->mem_c->ram_lower = 0x00 * 0x400;
	lpCPU->mem_c->ram_upper = 0x00 * 0x400 + 0x3FF;
	lpCPU->mem_c->banks = lpCPU->mem_c->normal_banks;
	lpCPU->mem_c->boot_mapped = false;
	lpCPU->mem_c->hasChangedPage0 = false;
#ifdef WITH_REVERSE
	lpCPU->prev_instruction = lpCPU->prev_instruction_list;
	memset(lpCPU->prev_instruction_list, 0, sizeof(lpCPU->prev_instruction_list));
	lpCPU->reverse_instr = 0;
#endif
	switch (lpCPU->pio.model) {
		case TI_81: {
			bank_state_t banks[5] = {
				{lpCPU->mem_c->flash, 						0, 		false,	false, 	false},
				{lpCPU->mem_c->flash+0x1*PAGE_SIZE,			0x1, 	false, 	false, 	false},
				{lpCPU->mem_c->flash+0x1*PAGE_SIZE,			0x1, 	false, 	false, 	false},
				{lpCPU->mem_c->ram,							0,		false,	true,	false},
				{nullptr,								0,		false,	false,	false}
			};
			memcpy(lpCPU->mem_c->normal_banks, banks, sizeof(banks));
			break;
		}
		case TI_82:
		case TI_83: {
			bank_state_t banks[5] = {
				{lpCPU->mem_c->flash, 						0, 		false,	false, 	false},
				{lpCPU->mem_c->flash+0x00*PAGE_SIZE, 		0x00, 	false, 	false, 	false},
				{lpCPU->mem_c->ram+0x01*PAGE_SIZE,		 	0x01, 	false, 	true, 	false},
				{lpCPU->mem_c->ram,							0,		false,	true,	false},
				{nullptr,										0,		false,	false,	false}
			};
			memcpy(lpCPU->mem_c->normal_banks, banks, sizeof(banks));
			break;
		}
		case TI_85:
		case TI_86: {
			bank_state_t banks[5] = {
				{lpCPU->mem_c->flash, 						0, 		false,	false, 	false},
				{lpCPU->mem_c->flash + 0x0F * PAGE_SIZE,	0x0F, 	false, 	false, 	false},
				{lpCPU->mem_c->flash,						0, 		false, 	false, 	false},
				{lpCPU->mem_c->ram,							0,		false,	true,	false},
				{nullptr,										0,		false,	false,	false}
			};

			memcpy(lpCPU->mem_c->normal_banks, banks, sizeof(banks));
			break;
		}
		case TI_73:
		case TI_83P: {
			/*bank_state_t banks[5] = {
				{lpCPU->mem_c->flash, 						0, 		false,	false, 	false},
				{lpCPU->mem_c->flash + 0x1f * PAGE_SIZE,	0x1f, 	false, 	false, 	false},
				{lpCPU->mem_c->flash + 0x1f * PAGE_SIZE,	0x1f, 	false, 	false, 	false},
				{lpCPU->mem_c->ram,							0,		false,	true,	false},
				{nullptr,										0,		false,	false,	false}
			};
			lpCPU->pc = 0x4000;*/
			memset(lpCPU->mem_c->protected_page, 0, sizeof(lpCPU->mem_c->protected_page));
			lpCPU->mem_c->protected_page_set = 0;
			/*	Address										page	write?	ram?	no exec?	*/
			bank_state_t banks[5] = {
				{lpCPU->mem_c->flash +  0x1f * PAGE_SIZE, 	0x1f, 	false,	false, 	false},
				{lpCPU->mem_c->flash,						0,		false, 	false, 	false},
				{lpCPU->mem_c->flash,						0,	 	false, 	false, 	false},
				{lpCPU->mem_c->ram,							0,		false,	true,	false},
				{nullptr,										0,		false,	false,	false}
			};
			memcpy(lpCPU->mem_c->normal_banks, banks, sizeof(banks));
			break;
		}
		case TI_83PSE:
		case TI_84PSE: {
			/*	Address										page	write?	ram?	no exec?	*/
			bank_state_t banks[5] = {
				{lpCPU->mem_c->flash +  0x7f * PAGE_SIZE, 	0x7f, 	false,	false, 	false},
				{lpCPU->mem_c->flash,						0,		false, 	false, 	false},
				{lpCPU->mem_c->flash,						0,	 	false, 	false, 	false},
				{lpCPU->mem_c->ram,							0,		false,	true,	false},
				{nullptr,										0,		false,	false,	false}
			};

			memcpy(lpCPU->mem_c->normal_banks, banks, sizeof(banks));
			break;
		}
		case TI_84P: {
			/*	Address										page	write?	ram?	no exec?	*/
			bank_state_t banks[5] = {
				{lpCPU->mem_c->flash + 0x3f * PAGE_SIZE,	0x3f,	false,	false, 	false},
				{lpCPU->mem_c->flash,						0,	 	false, 	false, 	false},
				{lpCPU->mem_c->flash,						0, 		false, 	false, 	false},
				{lpCPU->mem_c->ram,							0,		false,	true,	false},
				{nullptr,										0,		false,	false,	false}
			};
			memcpy(lpCPU->mem_c->normal_banks, banks, sizeof(banks));
			break;
		}
	}
	return 0;
}

int calc_run_frame(CALC* lpCalc) {
	uint64_t end_time = lpCalc->timer_c.freq / FPS;
	uint64_t cpu_sync = tc_tstates(&lpCalc->timer_c) + end_time - lpCalc->time_error;

	while(lpCalc->running) {
		CPU_step(&lpCalc->cpu);

		if (tc_tstates(&lpCalc->timer_c) >= cpu_sync) {
			lpCalc->time_error = (time_t)(tc_tstates((&lpCalc->timer_c)) - cpu_sync);
			return 0;
		}
	}

	return 0;
}

int calc_run_tstates(CALC* lpCalc, time_t tstates) {
	uint64_t time_end = tc_tstates(&lpCalc->timer_c) + tstates - lpCalc->time_error;

	while (lpCalc->running) {
		if (check_break(&lpCalc->mem_c, addr_to_waddr(&lpCalc->mem_c, lpCalc->cpu.pc)) & 1) {
#ifdef WINVER
			lpCalc->running = false;
			bank_t *bank = &lpCalc->mem_c.banks[mc_bank(lpCalc->cpu.pc)];

			Z80_info_t z[2];
			disassemble(lpCalc, REGULAR, addr_to_waddr(lpCalc->cpu.mem_c, lpCalc->cpu.pc), 1, z);

			if (lpCalc->pCalcNotify != nullptr) {
				
				bank_t *bank = &lpCalc->mem_c.banks[mc_bank(lpCalc->cpu.pc)];
				CCalcAddress *pCalcAddress = new CComObject<CCalcAddress>();
				pCalcAddress->Initialize(lpCalc->pWabbitemu, bank->ram, bank->page, lpCalc->cpu.pc);
				pCalcAddress->AddRef();
				lpCalc->pCalcNotify->Breakpoint(pCalcAddress);
			} else {
#endif
#ifndef MACVER
				lpCalc->breakpoint_callback(lpCalc);
#else
				printf("hit a breakpoint in run tstates\n");
				lpCalc->running = false; 
				lpCalc->breakpoint_callback(lpCalc,lpCalc->breakpoint_owner);
#endif
#ifdef WINVER
			}
#endif
			return 0;
		}
		uint64_t oldTStates;
		uint16_t oldPC;
		if (lpCalc->profiler.running) {
			oldTStates = tc_tstates(&lpCalc->timer_c);
			oldPC = lpCalc->cpu.pc % PAGE_SIZE;
		}
		if (link_hub_count > 1) {
			CPU_connected_step(&lpCalc->cpu);
			if (lpCalc->cpu.is_link_instruction) {
				lpCalc->time_error = (time_t)(tc_tstates((&lpCalc->timer_c)) - time_end);
				calc_waiting_link++;
				break;
			}
		} else {
			CPU_step(&lpCalc->cpu);
		}
		if (lpCalc->profiler.running) {
			uint64_t time = tc_tstates(&lpCalc->timer_c) - oldTStates;
			lpCalc->profiler.totalTime += time;
			bank_t bank = lpCalc->cpu.mem_c->banks[mc_bank(oldPC)];
			if (bank.ram) {
				lpCalc->profiler.ram_data[bank.page][oldPC / lpCalc->profiler.blockSize] += (long) time;
			} else {
				lpCalc->profiler.flash_data[bank.page][oldPC / lpCalc->profiler.blockSize] += (long) time;
			}
		}
		if (tc_tstates((&lpCalc->timer_c)) >= time_end) {
			lpCalc->time_error = (time_t)(tc_tstates((&lpCalc->timer_c)) - time_end);
			break;
		}
	}

	return 0;
}

bool calc_start_screenshot(calc_t *calc, const char *filename)
{
	if (gif_write_state == GIF_IDLE)
	{
		gif_write_state = GIF_START;
#ifdef _WINDOWS
		StringCbCopy(gif_file_name, PATH_MAX, filename);
#else
		_tcscpy_s(gif_file_name, filename);
#endif
		return true;
	}
	else
	{
		return false;
	}
}

void calc_stop_screenshot(CALC* calc)
{
	gif_write_state = GIF_END;
}

void calc_pause_linked() {
	for (int i = 0; i < MAX_CALCS; i++) {
		if (calcs[i].active && link_connected_hub(i)) {
			calcs[i].running = false;
		}
	}
}

void calc_unpause_linked() {
	for (int i = 0; i < MAX_CALCS; i++) {
		if (calcs[i].active && link_connected_hub(i)) {
			calcs[i].running = true;
		}
	}
}

#define FRAME_SUBDIVISIONS	(1024)
int calc_run_all(void) {
	int i, j, active_calc = -1;
	bool calc_waiting = false;

	for (i = 0; i < FRAME_SUBDIVISIONS; i++) {
		link_hub[MAX_CALCS]->host = 0;
		for (j = 0; j < MAX_CALCS; j++) {
			char hostVal = 0;
			for (int k = 0; k < MAX_CALCS; k++) {
				if (link_hub[k] != nullptr && link_hub[k]->host) {
					hostVal |= link_hub[k]->host;
					calc_waiting |= link_hub[k]->hasChanged;
				}
			}
			if (hostVal != link_hub[MAX_CALCS]->host) {
				link_hub[MAX_CALCS]->host = hostVal;
				calc_waiting = true;
				for (int k = 0; k < MAX_CALCS; k++) {
					if (link_hub[k]) {
						link_hub[k]->hasChanged = true;
						link_hub[k]->changedTime = calcs[k].timer_c.tstates;
					}
				}
			}
			if (calcs[j].active) {
				/*if (link_hub[j] != nullptr && (!link_hub[MAX_CALCS]->host || link_hub[MAX_CALCS]->host != link_hub[j]->host)
					&& calcs[j].cpu.is_link_instruction || ((int) (calcs[j].cpu.linking_time - calcs[j].cpu.timer_c->tstates) >= 100000)) {
					calcs[j].cpu.is_link_instruction = false;
					calcs[j].cpu.linking_time = 0;
					CPU_step(&calcs[j].cpu);
				}*/
				if (calcs[j].cpu.is_link_instruction && calcs[j].cpu.pio.link->changedTime - calcs[j].timer_c.tstates >= 100000) {
					calcs[j].cpu.is_link_instruction = false;
					calcs[j].cpu.pio.link->changedTime = 0;
					calcs[j].cpu.pio.link->hasChanged = false;
					CPU_step(&calcs[j].cpu);
				}
				active_calc = j;
				int time = (int)((int64_t) calcs[j].speed * calcs[j].timer_c.freq / FPS / 100) / FRAME_SUBDIVISIONS;
				if (!calcs[j].cpu.is_link_instruction || !calc_waiting || calcs[j].cpu.pio.link->hasChanged == true) {
					calc_run_tstates(&calcs[j], time);
				} /*else {
					calcs[j].cpu.linking_time += time;
				}*/
			}
		}

		if (link_hub_count > 1 && calc_waiting_link >= link_hub_count) {
			for (int k = 0; k < MAX_CALCS; k++) {
				if (calcs[k].cpu.is_link_instruction) {
					calcs[k].cpu.is_link_instruction = false;
					CPU_step(&calcs[k].cpu);
				}
			}
			calc_waiting_link = 0;
		}

		//this code handles screenshoting if were actually taking screenshots right now
		if (active_calc >= 0 && !calc_waiting_link && calcs[active_calc].cpu.timer_c != nullptr && calcs[active_calc].cpu.pio.lcd != nullptr &&
				((tc_elapsed(calcs[active_calc].cpu.timer_c) - calcs[active_calc].cpu.pio.lcd->lastgifframe) >= 0.01)) {
			handle_screenshot();
			calcs[active_calc].cpu.pio.lcd->lastgifframe += 0.01;
		}
	}

	return 0;
}

void link_step(CPU_t *cpu) {

}

void port_debug_callback(void *arg1, void *arg2) {
	CPU_t *cpu = (CPU_t *) arg1;
	//device_t *dev = (device_t *) arg2;
	CALC* lpCalc = calc_from_cpu(cpu);
#ifdef MACVER
	lpCalc->breakpoint_callback(lpCalc, lpCalc->breakpoint_owner);
#else
	lpCalc->breakpoint_callback(lpCalc);
#endif
}

void mem_debug_callback(void *arg1) {
	CPU_t *cpu = (CPU_t *) arg1;
	CALC* lpCalc = calc_from_cpu(cpu);
#ifdef MACVER
	lpCalc->breakpoint_callback(lpCalc, lpCalc->breakpoint_owner);
#else
	lpCalc->breakpoint_callback(lpCalc);
#endif
}

#ifdef WITH_BACKUPS
void do_backup(CALC* lpCalc) {
	if (!lpCalc->running)
		return;
	int slot = lpCalc->slot;
	if (number_backup > MAX_BACKUPS) {
		debugger_backup* oldestBackup = backups[slot];
		while(oldestBackup->prev != nullptr)
			oldestBackup = oldestBackup->prev;
		oldestBackup->next->prev = nullptr;
		free_backup(oldestBackup);
	}
	debugger_backup *backup = (debugger_backup *) malloc(sizeof(debugger_backup));
	backup->save = SaveSlot(lpCalc);
	backup->next = nullptr;
	backup->prev = backups[slot];
	if (backups[slot] != nullptr)
		backups[slot]->next = backup;
	backups[slot] = backup;
	number_backup++;
}
void restore_backup(int index, CALC* lpCalc) {
	int slot = lpCalc->slot;
	debugger_backup* backup = backups[slot];
	while (index > 0) {
		if (backup->prev == nullptr)
			break;
		backup = backup->prev;
		free_backup(backup->next);
		index--;
	}
	//shouldn't happen
	if (backup != nullptr)
		LoadSlot(backup->save, lpCalc);
	backups[slot] = backup;
}

void init_backups() {
	int i;
	number_backup = 0;
	for(i = 0; i < MAX_CALCS; i++)
		backups[i] = nullptr;
}

void free_backup(debugger_backup* backup) {
	if (backup == nullptr)
		return;
	FreeSave(backup->save);
	free(backup);
	number_backup--;
}

/*
 * Frees all backups from memory
 */
void free_backups(CALC* lpCalc) {
	int slot = lpCalc->slot;
	debugger_backup *backup_prev, *backup = backups[slot];
	if (backup == nullptr)
		return;
	do {
		backup_prev = backup->prev;
		free_backup(backup);
		backup = backup_prev;
	} while(backup != nullptr);
	backups[slot] = nullptr;
	number_backup = 0;
}
#endif

int calc_run_seconds(CALC* lpCalc, double seconds) {
	time_t time = (time_t ) (seconds * 1000);
	return calc_run_timed(lpCalc, time);
}


// ticks
int calc_run_timed(CALC* lpCalc, time_t time) {
	int frames = (int) time / TPF;

	int speed_backup = lpCalc->speed;

	lpCalc->speed = MAX_SPEED;
	while (frames--) 
		calc_run_frame(lpCalc);
	lpCalc->speed = speed_backup;
	return 0;
}

CALC* calc_from_cpu(CPU_t *cpu) {
	int i;
	for (i = 0; i < MAX_CALCS; i++) {
		if (&calcs[i].cpu == cpu)
			return &calcs[i];
	}
	return nullptr;
}

CALC* calc_from_memc(memc *memc) {
	for (int i = 0; i < MAX_CALCS; i++) {
		if (calcs[i].cpu.mem_c == memc) {
			return &calcs[i];
		}
	}
	return nullptr;
}

#ifdef WINVER
CALC* calc_from_hwnd(HWND hwnd) {
	if (hwnd == nullptr)
		return nullptr;

	int slot;
	for (slot = 0; slot < MAX_CALCS; slot++) {
		if (calcs[slot].active) {
			if (hwnd == calcs[slot].hwndFrame ||
				hwnd == calcs[slot].hwndLCD ||
				hwnd == calcs[slot].hwndStatusBar ||
				hwnd == calcs[slot].hwndSmallClose ||
				hwnd == calcs[slot].hwndSmallMinimize) {
				return &calcs[slot];
			}
		}
	}
	return nullptr;
}
#endif
