#include "../core/core.hpp"
#include "savestate.hpp"
#include "../hardware/lcd.hpp"
#include "../hardware/link.hpp"
#include "../interface/calc.hpp"
#include "../hardware/83psehw.hpp"
#include "fileutilities.hpp"

extern int def(FILE *, FILE *, int);
extern int inf(FILE *, FILE *);

bool cmpTags(const char *str1, const char *str2) {
	int i;
	for(i = 0; i < 4; i++) {
		if (str1[i] != str2[i]) return false;
	}
	return true;
}

int fputi(unsigned int integer, FILE* stream) {
	int r;
	r = fputc(integer & 0xFF, stream);
	if (r == EOF) return EOF;
	r = fputc((integer >> 8) & 0xFF, stream);
	if (r == EOF) return EOF;
	r = fputc((integer >> 16) & 0xFF, stream);
	if (r == EOF) return EOF;
	r = fputc((integer >> 24) & 0xFF, stream);
	return r;
}
	
int fgeti(FILE* stream) {
	int r;
	r  = fgetc(stream);
	r += fgetc(stream)<<8;
	r += fgetc(stream)<<16;
	r += fgetc(stream)<<24;
	return r;
}

SAVESTATE_t* CreateSave(const char *author, const char *comment , int model) {
	SAVESTATE_t* save = (SAVESTATE_t*) malloc(sizeof(SAVESTATE_t));
	if (!save) return nullptr;

	save->version_major = CUR_MAJOR;
	save->version_minor = CUR_MINOR;
	save->version_build = CUR_BUILD;

	memset(save->author, 0, sizeof(save->author));
	memset(save->comment, 0, sizeof(save->comment));
	strncpy(save->author, author, sizeof(save->author));
	strncpy(save->comment, comment, sizeof(save->comment));
	
	save->model = model;
	save->chunk_count = 0;
	
	uint32_t i;
	for(i = 0; i < NumElm(save->chunks); i++) {
		save->chunks[i] = nullptr;
	}
	return save;
}

void ClearSave(SAVESTATE_t* save) {
	int i;
	
	if (save == nullptr) return;

	for(i = 0; i < save->chunk_count; i++) {
		if (save->chunks[i]) {
			if (save->chunks[i]->data) {
				free(save->chunks[i]->data);
				save->chunks[i]->data = nullptr;
			}
			free(save->chunks[i]);

			save->chunks[i] = nullptr;
		}
	}

}

void FreeSave(SAVESTATE_t* save) {
	ClearSave(save);
	free(save);
	save = nullptr;
}

CHUNK_t* FindChunk(SAVESTATE_t* save, const char *tag) {
	int i;
	for(i = 0; i < save->chunk_count; i++) {
		if (cmpTags(save->chunks[i]->tag, tag) == true) {
			save->chunks[i]->pnt = 0;
			return save->chunks[i];
		}
	}	
	return nullptr;
}

CHUNK_t* NewChunk(SAVESTATE_t* save, const char *tag) {
	int chunk = save->chunk_count;

	if (FindChunk(save, tag) != nullptr) {
		printf("Error: chunk '%s' already exists", tag);
		return nullptr;
	}

	if (save->chunks[chunk] != nullptr) {
		puts("Error new chunk was not null.");
	}
	save->chunks[chunk] = (CHUNK_t *) malloc(sizeof(CHUNK_t));
	if (!save->chunks[chunk]) {
		puts("Chunk could not be created");
		return nullptr;
	}

	save->chunks[chunk]->tag[0]	= tag[0];
	save->chunks[chunk]->tag[1]	= tag[1];
	save->chunks[chunk]->tag[2]	= tag[2];
	save->chunks[chunk]->tag[3]	= tag[3];
	save->chunks[chunk]->size	= 0;
	save->chunks[chunk]->data	= nullptr;
	save->chunks[chunk]->pnt	= 0;
	save->chunk_count++;
	return save->chunks[chunk];
}

bool DelChunk(SAVESTATE_t *save, const char *tag) {
	int i;
	for(i = 0; i < save->chunk_count; i++) {
		if (cmpTags(save->chunks[i]->tag, tag) == true) {
			if (save->chunks[i]->data) free(save->chunks[i]->data);
			if (save->chunks[i]) free(save->chunks[i]);
			for(; i < (save->chunk_count - 1); i++) {
				save->chunks[i] = save->chunks[i+1];
			}
			save->chunks[i] = nullptr;
			save->chunk_count--;
			return true;
		}
	}	
	return false;
}


bool CheckPNT(CHUNK_t* chunk) {
	if (chunk->size < chunk->pnt) {
		return false;
	}
	return true;
}

bool WriteChar(CHUNK_t* chunk, char value) {
	unsigned char * tmppnt;
	tmppnt = (unsigned char *) realloc(chunk->data, chunk->size + sizeof(char));
	if (tmppnt == nullptr) {
		return false;
	}
	chunk->data = tmppnt;
	chunk->data[chunk->size] = value;
	chunk->size += sizeof(char);
	return true;
}


bool WriteShort(CHUNK_t* chunk, uint16_t value) {
	int i;
	unsigned char  *tmppnt;
	unsigned char  *pnt = (unsigned char *)(&value);
	tmppnt = (unsigned char *) realloc(chunk->data,chunk->size + sizeof(value));
	if (tmppnt == nullptr) {
		return false;
	}
	chunk->data = tmppnt;
#ifdef __BIG_ENDIAN__
	for(i=sizeof(value)-1;i>=0;i--) {
#else
	for(i=0;i<sizeof(value);i++) {
#endif	
		chunk->data[i+chunk->size] = *pnt++;
	}
	chunk->size += sizeof(value);
	return true;
}
bool WriteInt(CHUNK_t* chunk, uint32_t value) {
	int i;
	unsigned char *tmppnt;
	unsigned char *pnt = (unsigned char *)(&value);
	tmppnt = (unsigned char *) realloc(chunk->data,chunk->size + sizeof(value));
	if (tmppnt == nullptr) {
		return false;
	}
	chunk->data = tmppnt;
#ifdef __BIG_ENDIAN__
	for(i=sizeof(value)-1;i>=0;i--) {
#else
	for(i = 0;i < sizeof(value); i++) {
#endif
		chunk->data[i+chunk->size] = *pnt++;
	}
	chunk->size += sizeof(uint32_t);
	return true;
}

bool WriteLong(CHUNK_t* chunk, uint64_t value) {
	int i;
	unsigned char *tmppnt;
	unsigned char *pnt = (unsigned char *)(&value);
	tmppnt = (unsigned char  *) realloc(chunk->data, chunk->size + sizeof(value));
	if (tmppnt == nullptr) {
		return false;
	}
	chunk->data = tmppnt;
#ifdef __BIG_ENDIAN__
	for(i=sizeof(value)-1;i>=0;i--) {
#else
	for(i = 0; i < sizeof(value); i++) {
#endif
		chunk->data[i+chunk->size] = *pnt++;
	}
	chunk->size += sizeof(value);
	return true;
}

bool WriteFloat(CHUNK_t* chunk, float value) {
	int i;
	unsigned char *tmppnt;
	unsigned char *pnt = (unsigned char *)(&value);
	tmppnt = (unsigned char *) realloc(chunk->data,chunk->size + sizeof(value));
	if (tmppnt == nullptr) {
		return false;
	}
	chunk->data = tmppnt;
#ifdef __BIG_ENDIAN__
	for(i=sizeof(value)-1;i>=0;i--) {
#else
	for(i = 0; i < sizeof(value); i++) {
#endif
		chunk->data[i+chunk->size] = *pnt++;
	}
	chunk->size += sizeof(value);
	return true;
}	
bool WriteDouble(CHUNK_t* chunk, double value) {
	int i;
	unsigned char *tmppnt;
	unsigned char *pnt = (unsigned char *)(&value);
	tmppnt = (unsigned char *) realloc(chunk->data,chunk->size + sizeof(value));
	if (tmppnt == nullptr) {
		return false;
	}
	chunk->data = tmppnt;
#ifdef __BIG_ENDIAN__
	for(i=sizeof(value)-1;i>=0;i--) {
#else
	for(i = 0;i < sizeof(value); i++) {
#endif
		chunk->data[i+chunk->size] = *pnt++;
	}
	chunk->size += sizeof(value);
	return true;
}

bool WriteBlock(CHUNK_t* chunk, unsigned char *pnt, int length) {
	int i;
	unsigned char *tmppnt;
	tmppnt = (unsigned char *) realloc(chunk->data,chunk->size+length);
	if (tmppnt == nullptr) {
		return false;
	}
	chunk->data = tmppnt;
	for(i = 0; i < length; i++) {
		chunk->data[i+chunk->size] = pnt[i];
	}
	chunk->size += length;
	return true;
}		
	

	
unsigned char ReadChar(CHUNK_t* chunk, bool *valOK = nullptr) {
	unsigned char value;
	value = chunk->data[chunk->pnt];
	chunk->pnt += sizeof(unsigned char);
	if (valOK) {
		*valOK = CheckPNT(chunk);
	}
	return value;
}

unsigned short ReadShort(CHUNK_t* chunk, bool *valOK = nullptr) {
	int i;
	uint16_t value;
	unsigned char *pnt = (unsigned char *)(&value);
#ifdef __BIG_ENDIAN__
	for(i = sizeof(value) - 1; i >= 0; i--) {
#else
	for(i = 0;i < sizeof(value); i++) {
#endif
		*pnt++ = chunk->data[i+chunk->pnt];
	}
	chunk->pnt += sizeof(value);
	if (valOK) {
		*valOK = CheckPNT(chunk);
	}
	return value;
}

unsigned int ReadInt(CHUNK_t* chunk, bool *valOK = nullptr) {
	int i;
	uint32_t value;
	unsigned char *pnt = (unsigned char *)(&value);
#ifdef __BIG_ENDIAN__
	for(i = sizeof(value) - 1; i >= 0; i--) {
#else
	for(i = 0; i < sizeof(value); i++) {
#endif
		*pnt++ = chunk->data[i+chunk->pnt];
	}
	chunk->pnt += sizeof(value);
	if (valOK) {
		*valOK = CheckPNT(chunk);
	}
	return value;
}

float ReadFloat(CHUNK_t* chunk) {
	int i;
	float value;
	unsigned char *pnt = (unsigned char *)(&value);
#ifdef __BIG_ENDIAN__
	for(i=sizeof(float)-1;i>=0;i--) {
#else
	for(i = 0; i < sizeof(float); i++) {
#endif
		*pnt++ = chunk->data[i+chunk->pnt];
	}
	chunk->pnt += sizeof(float);
	CheckPNT(chunk);
	return value;
}


double ReadDouble(CHUNK_t* chunk) {
	int i;
	double value;
	unsigned char *pnt = (unsigned char *)(&value);
#ifdef __BIG_ENDIAN__
	for(i=sizeof(double)-1;i>=0;i--) {
#else
	for(i = 0; i < sizeof(double); i++) {
#endif
		*pnt++ = chunk->data[i+chunk->pnt];
	}
	chunk->pnt += sizeof(double);
	CheckPNT(chunk);
	return value;
}

uint64_t ReadLong(CHUNK_t* chunk)
{
	int i;
	uint64_t value;
	unsigned char *pnt = (unsigned char *)(&value);
#ifdef __BIG_ENDIAN__
	for(i = sizeof(value) - 1; i >= 0; i--) {
#else
	for(i = 0;i < sizeof(value); i++) {
#endif
		*pnt++ = chunk->data[i+chunk->pnt];
	}
	chunk->pnt += sizeof(value);
	CheckPNT(chunk);
	return value;
}

void ReadBlock(CHUNK_t* chunk, unsigned char *pnt, int length) {
	int i;
	for(i = 0; i < length; i++) {
		pnt[i] = chunk->data[i+chunk->pnt];
	}
	chunk->pnt += length;
	CheckPNT(chunk);
}


void SaveCPU(SAVESTATE_t* save, CPU_t* cpu) {
	int i;
	if (!cpu) return;
	CHUNK_t* chunk = NewChunk(save,CPU_tag);
	
	WriteChar(chunk, cpu->a);
	WriteChar(chunk, cpu->f);
	WriteChar(chunk, cpu->b);
	WriteChar(chunk, cpu->c);
	WriteChar(chunk, cpu->d);
	WriteChar(chunk, cpu->e);
	WriteChar(chunk, cpu->h);
	WriteChar(chunk, cpu->l);
	
	WriteChar(chunk, cpu->ap);
	WriteChar(chunk, cpu->fp);
	WriteChar(chunk, cpu->bp);
	WriteChar(chunk, cpu->cp);
	WriteChar(chunk, cpu->dp);
	WriteChar(chunk, cpu->ep);
	WriteChar(chunk, cpu->hp);
	WriteChar(chunk, cpu->lp);
	
	WriteChar(chunk, cpu->ixl);
	WriteChar(chunk, cpu->ixh);
	WriteChar(chunk, cpu->iyl);
	WriteChar(chunk, cpu->iyh);

	WriteShort(chunk, cpu->pc);
	WriteShort(chunk, cpu->sp);

	WriteChar(chunk, cpu->i);
	WriteChar(chunk, cpu->r);
	WriteChar(chunk, cpu->bus);
	
	WriteInt(chunk, cpu->imode);

	WriteInt(chunk, cpu->interrupt);
	WriteInt(chunk, cpu->ei_block);
	WriteInt(chunk, cpu->iff1);
	WriteInt(chunk, cpu->iff2);
	WriteInt(chunk, cpu->halt);
	
	WriteInt(chunk, cpu->read);
	WriteInt(chunk, cpu->write);
	WriteInt(chunk, cpu->output);
	WriteInt(chunk, cpu->input);
	WriteInt(chunk, cpu->prefix);

	
	/* pio */
	for(i = 0; i < 256; i++) {
		interrupt_t *val = &cpu->pio.interrupt[i];
		WriteInt(chunk, val->interrupt_val);
		WriteInt(chunk, val->skip_factor);
		WriteInt(chunk, val->skip_count);
	}
}
	
void SaveMEM(SAVESTATE_t* save, memc* mem) {
	int i;
	if (!mem) return;
	CHUNK_t *chunk = NewChunk(save, MEM_tag);

	WriteInt(chunk, mem->flash_size);
	WriteInt(chunk, mem->flash_pages);
	WriteInt(chunk, mem->ram_size);
	WriteInt(chunk, mem->ram_pages);
	WriteInt(chunk, mem->step);
	WriteChar(chunk, mem->cmd);
	
	WriteInt(chunk, mem->boot_mapped);
	WriteInt(chunk, mem->flash_locked);
	WriteInt(chunk, mem->flash_version);	

	for(i = 0; i < 5; i++) {
		WriteInt(chunk, mem->normal_banks[i].page);
		WriteInt(chunk, mem->normal_banks[i].read_only);
		WriteInt(chunk, mem->normal_banks[i].ram);
		WriteInt(chunk, mem->normal_banks[i].no_exec);
	}
	
	WriteInt(chunk, mem->read_OP_flash_tstates);
	WriteInt(chunk, mem->read_NOP_flash_tstates);
	WriteInt(chunk, mem->write_flash_tstates);
	WriteInt(chunk, mem->read_OP_ram_tstates);
	WriteInt(chunk, mem->read_NOP_ram_tstates);
	WriteInt(chunk, mem->write_ram_tstates);
	
	WriteInt(chunk, mem->flash_upper);
	WriteInt(chunk, mem->flash_lower);

	chunk = NewChunk(save, ROM_tag);
	WriteBlock(chunk, mem->flash, mem->flash_size);

	chunk = NewChunk(save, RAM_tag);
	WriteBlock(chunk, mem->ram, mem->ram_size);
	
	chunk = NewChunk(save, REMAP_tag);
	WriteInt(chunk, mem->port27_remap_count);
	WriteInt(chunk, mem->port28_remap_count);

	chunk = NewChunk(save, RAM_LIMIT_tag);
	WriteInt(chunk, mem->ram_upper);
	WriteInt(chunk, mem->ram_lower);

	int count = 0;
	chunk = NewChunk(save, FLASH_BREAKS_tag);
	for (int i = 0; i < mem->flash_size; i++)
	{
		if (mem->flash_break[i])
		{
			count++;
			WriteInt(chunk, i);
			WriteInt(chunk, mem->flash_break[i]);
		}
	}
	chunk = NewChunk(save, NUM_FLASH_BREAKS_tag);
	WriteInt(chunk, count);

	count = 0;
	chunk = NewChunk(save, RAM_BREAKS_tag);
	for (int i = 0; i < mem->ram_size; i++)
	{
		if (mem->ram_break[i])
		{
			count++;
			WriteInt(chunk, i);
			WriteInt(chunk, mem->ram_break[i]);
		}
	}
	chunk = NewChunk(save, NUM_RAM_BREAKS_tag);
	WriteInt(chunk, count);
}

void SaveTIMER(SAVESTATE_t *save, timerc *time) {
	if (!time) return;
	CHUNK_t* chunk = NewChunk(save,TIMER_tag);
	WriteLong(chunk, time->tstates);
	WriteLong(chunk, time->freq);
	WriteDouble(chunk, tc_elapsed(time));
	WriteDouble(chunk, time->lasttime);
}

void SaveLINK(SAVESTATE_t* save, link_t* link) {
	CHUNK_t* chunk = NewChunk(save, LINK_tag);
	WriteChar(chunk, link->host);
}

void SaveSTDINT(SAVESTATE_t* save, STDINT_t *stdint) {
	int i;
	if (!stdint) return;
	CHUNK_t* chunk = NewChunk(save, STDINT_tag);
	WriteChar(chunk, stdint->intactive);

	WriteDouble(chunk, stdint->lastchk1);
	WriteDouble(chunk, stdint->timermax1);
	WriteDouble(chunk, stdint->lastchk2);
	WriteDouble(chunk, stdint->timermax2);
	for(i = 0; i < 4; i++) {
		WriteDouble(chunk, stdint->freq[i]);
	}
	WriteInt(chunk, stdint->mem);
	WriteInt(chunk, stdint->xy);
}

void SaveSE_AUX(SAVESTATE_t* save, SE_AUX_t *se_aux) {
	int i;
	if (!se_aux) return;
	CHUNK_t* chunk = NewChunk(save, SE_AUX_tag);
	
	if (save->model > TI_83P) {
		WriteChar(chunk, se_aux->clock.enable);
		WriteInt(chunk, (uint32_t)se_aux->clock.set);
		WriteInt(chunk, (uint32_t)se_aux->clock.base);
		WriteDouble(chunk, se_aux->clock.lasttime);
	
		for(i = 0; i < 7; i++) {
			WriteChar(chunk, se_aux->delay.reg[i]);
		}
	
		for(i = 0; i < 6; i++) {
			WriteInt(chunk, se_aux->md5.reg[i]);
		}
		WriteChar(chunk, se_aux->md5.s);
		WriteChar(chunk, se_aux->md5.mode);
	}
	
	
	WriteChar(chunk, se_aux->linka.link_enable);
	WriteChar(chunk, se_aux->linka.in);
	WriteChar(chunk, se_aux->linka.out);
	WriteChar(chunk, se_aux->linka.working);
	WriteInt(chunk, se_aux->linka.receiving);
	WriteInt(chunk, se_aux->linka.read);
	WriteInt(chunk, se_aux->linka.ready);
	WriteInt(chunk, se_aux->linka.error);
	WriteInt(chunk, se_aux->linka.sending);
	WriteDouble(chunk, se_aux->linka.last_access);
	WriteInt(chunk, se_aux->linka.bit);

	if (save->model < TI_83PSE)
		return;
	
	WriteDouble(chunk, se_aux->xtal.lastTime);
	WriteLong(chunk, se_aux->xtal.ticks);
	for(i = 0; i < 3; i++) {
		WriteLong(chunk, se_aux->xtal.timers[i].lastTstates);
		WriteDouble(chunk, se_aux->xtal.timers[i].lastTicks);
		WriteDouble(chunk, se_aux->xtal.timers[i].divsor);
		WriteInt(chunk, se_aux->xtal.timers[i].loop);
		WriteInt(chunk, se_aux->xtal.timers[i].interrupt);
		WriteInt(chunk, se_aux->xtal.timers[i].underflow);
		WriteInt(chunk, se_aux->xtal.timers[i].generate);
		WriteInt(chunk, se_aux->xtal.timers[i].active);
		WriteChar(chunk, se_aux->xtal.timers[i].clock);
		WriteChar(chunk, se_aux->xtal.timers[i].count);
		WriteChar(chunk, se_aux->xtal.timers[i].max);
	}
	WriteInt(chunk, se_aux->model_bits);
	chunk = NewChunk(save, USB_tag);
	WriteInt(chunk, se_aux->usb.USBLineState);
	WriteInt(chunk, se_aux->usb.USBEvents);
	WriteInt(chunk, se_aux->usb.USBEventMask);
	WriteInt(chunk, se_aux->usb.LineInterrupt);
	WriteInt(chunk, se_aux->usb.ProtocolInterrupt);
	WriteInt(chunk, se_aux->usb.ProtocolInterruptEnabled);
	WriteInt(chunk, se_aux->usb.DevAddress);
	WriteChar(chunk, se_aux->usb.Port4A);
	WriteChar(chunk, se_aux->usb.Port4C);
	WriteChar(chunk, se_aux->usb.Port54);
}

void SaveLCD(SAVESTATE_t* save, LCD_t* lcd) {
	if (!lcd) return;
	CHUNK_t* chunk = NewChunk(save, LCD_tag);

	WriteInt(chunk, lcd->active);
	WriteInt(chunk, lcd->word_len);
	WriteInt(chunk, lcd->x);
	WriteInt(chunk, lcd->y);
	WriteInt(chunk, lcd->z);
	WriteInt(chunk, lcd->cursor_mode);
	WriteInt(chunk, lcd->contrast);
	WriteInt(chunk, lcd->base_level);
	WriteBlock(chunk, lcd->display, DISPLAY_SIZE);

	WriteInt(chunk, lcd->front);
	WriteBlock(chunk, (unsigned char *) lcd->queue, LCD_MAX_SHADES * DISPLAY_SIZE);
	
	WriteInt(chunk, lcd->shades);
	WriteInt(chunk, lcd->mode);
	WriteDouble(chunk, lcd->time);
	WriteDouble(chunk, lcd->ufps);
	WriteDouble(chunk, lcd->ufps_last);
	WriteDouble(chunk, lcd->lastgifframe);
	WriteDouble(chunk, lcd->write_avg);
	WriteDouble(chunk, lcd->write_last);
}

SAVESTATE_t* SaveSlot(void *lpInput) {
	CALC* lpCalc = (CALC*) lpInput;
	SAVESTATE_t* save;
	bool runsave;
	if (lpCalc->active == false) return nullptr;

	runsave = lpCalc->running;
	lpCalc->running = false;
	
	save = CreateSave(("Revsoft"), ("Test save"), lpCalc->model);

	SaveCPU(save, &lpCalc->cpu);
	SaveMEM(save, &lpCalc->mem_c);
	SaveTIMER(save, &lpCalc->timer_c);
	SaveLCD(save, lpCalc->cpu.pio.lcd);
	SaveLINK(save, lpCalc->cpu.pio.link);
	SaveSTDINT(save, lpCalc->cpu.pio.stdint);
	SaveSE_AUX(save, lpCalc->cpu.pio.se_aux);

	lpCalc->running = runsave;
	return save;
}

void LoadCPU(SAVESTATE_t* save, CPU_t* cpu) {
	CHUNK_t* chunk = FindChunk(save, CPU_tag);
	chunk->pnt = 0;
	
	cpu->a = ReadChar(chunk);
	cpu->f = ReadChar(chunk);
	cpu->b = ReadChar(chunk);
	cpu->c = ReadChar(chunk);
	cpu->d = ReadChar(chunk);
	cpu->e = ReadChar(chunk);
	cpu->h = ReadChar(chunk);
	cpu->l = ReadChar(chunk);
	
	cpu->ap = ReadChar(chunk);
	cpu->fp = ReadChar(chunk);
	cpu->bp = ReadChar(chunk);
	cpu->cp = ReadChar(chunk);
	cpu->dp = ReadChar(chunk);
	cpu->ep = ReadChar(chunk);
	cpu->hp = ReadChar(chunk);
	cpu->lp = ReadChar(chunk);

	cpu->ixl = ReadChar(chunk);
	cpu->ixh = ReadChar(chunk);
	cpu->iyl = ReadChar(chunk);
	cpu->iyh = ReadChar(chunk);
	
	cpu->pc = ReadShort(chunk);
	cpu->sp = ReadShort(chunk);

	cpu->i = ReadChar(chunk);
	cpu->r = ReadChar(chunk);
	cpu->bus = ReadChar(chunk);

	cpu->imode = ReadInt(chunk);

	cpu->interrupt = ReadInt(chunk);
	cpu->ei_block = ReadInt(chunk);
	cpu->iff1 = ReadInt(chunk);
	cpu->iff2 = ReadInt(chunk);
	cpu->halt = ReadInt(chunk);
	
	cpu->read = ReadInt(chunk);
	cpu->write = ReadInt(chunk);
	cpu->output = ReadInt(chunk);
	cpu->input = ReadInt(chunk);
	
	cpu->prefix = ReadInt(chunk);
	int i;
	for(i = 0; i < 256; i++) {
		interrupt_t *val = &cpu->pio.interrupt[i];
		val->interrupt_val = ReadInt(chunk);
		val->skip_factor = ReadInt(chunk);
		val->skip_count = ReadInt(chunk);
	}
	
}



void LoadMEM(SAVESTATE_t* save, memc* mem) {
	int i;
	CHUNK_t* chunk = FindChunk(save, MEM_tag);
	chunk->pnt = 0;

	mem->flash_size	= ReadInt(chunk);
	mem->flash_pages= ReadInt(chunk);
	mem->ram_size	= ReadInt(chunk);
	mem->ram_pages	= ReadInt(chunk);
	mem->step		= ReadInt(chunk);
	mem->cmd		= ReadChar(chunk);
	mem->boot_mapped= ReadInt(chunk);
	mem->flash_locked= ReadInt(chunk);
	mem->flash_version = ReadInt(chunk);
	
	for(i = 0; i < 5; i++) {
		mem->normal_banks[i].page		= ReadInt(chunk);
		mem->normal_banks[i].read_only	= ReadInt(chunk);
		mem->normal_banks[i].ram		= ReadInt(chunk);
		mem->normal_banks[i].no_exec	= ReadInt(chunk);
		if (mem->normal_banks[i].ram) {
			mem->normal_banks[i].addr = mem->ram+(mem->normal_banks[i].page*PAGE_SIZE);
		} else {
			mem->normal_banks[i].addr = mem->flash+(mem->normal_banks[i].page*PAGE_SIZE);
		}
	}
	if (mem->boot_mapped) {
		update_bootmap_pages(mem);
		mem->banks = mem->bootmap_banks;
	} else
		mem->banks = mem->normal_banks;
	
	mem->read_OP_flash_tstates	= ReadInt(chunk);
	mem->read_NOP_flash_tstates	= ReadInt(chunk);
	mem->write_flash_tstates	= ReadInt(chunk);
	mem->read_OP_ram_tstates	= ReadInt(chunk);
	mem->read_NOP_ram_tstates	= ReadInt(chunk);
	mem->write_ram_tstates		= ReadInt(chunk);

	mem->flash_upper = ReadInt(chunk);
	mem->flash_lower = ReadInt(chunk);

	chunk = FindChunk(save, ROM_tag);
	chunk->pnt = 0;
	ReadBlock(chunk, (unsigned char *)mem->flash, mem->flash_size);
	
	chunk = FindChunk(save, RAM_tag);
	chunk->pnt = 0;
	ReadBlock(chunk, (unsigned char *)mem->ram, mem->ram_size);	

	
	chunk = FindChunk(save, REMAP_tag);
	if (chunk) {
		chunk->pnt = 0;
		mem->port27_remap_count = ReadInt(chunk);
		mem->port28_remap_count = ReadInt(chunk);
	}
	chunk = FindChunk(save, RAM_LIMIT_tag);
	if (chunk) {
		chunk->pnt = 0;
		mem->ram_upper = ReadInt(chunk);
		mem->ram_lower = ReadInt(chunk);
	}

	chunk = FindChunk(save, NUM_FLASH_BREAKS_tag);
	if (chunk) {
		int num_flash_breaks = ReadInt(chunk);
		chunk = FindChunk(save, FLASH_BREAKS_tag);
		if (chunk) {
			for (int i = 0; i < num_flash_breaks; i++)
			{
				int addr = ReadInt(chunk);
				waddr_t waddr;
				waddr.addr = addr % PAGE_SIZE;
				waddr.page = addr / PAGE_SIZE;
				waddr.is_ram = false;
				BREAK_TYPE type = (BREAK_TYPE) ReadInt(chunk);
				switch (type) {
				case MEM_READ_BREAK:
					set_mem_read_break(mem, waddr);
					break;
				case MEM_WRITE_BREAK:
					set_mem_read_break(mem, waddr);
					break;
				default:
					set_break(mem, waddr);
					break;
				}
			}
		}
	}

	chunk = FindChunk(save, NUM_RAM_BREAKS_tag);
	if (chunk) {
		int num_ram_breaks = ReadInt(chunk);
		chunk = FindChunk(save, FLASH_BREAKS_tag);
		if (chunk) {
			for (int i = 0; i < num_ram_breaks; i++)
			{
				bool valOk;
				int addr = ReadInt(chunk, &valOk);
				if (valOk) {
					waddr_t waddr;
					waddr.addr = addr % PAGE_SIZE;
					waddr.page = addr / PAGE_SIZE;
					waddr.is_ram = true;
					BREAK_TYPE type = (BREAK_TYPE) ReadInt(chunk);
					switch (type) {
					case MEM_READ_BREAK:
						set_mem_read_break(mem, waddr);
						break;
					case MEM_WRITE_BREAK:
						set_mem_read_break(mem, waddr);
						break;
					default:
						set_break(mem, waddr);
						break;
					}
				}
			}
		}
	}
}

void LoadTIMER(SAVESTATE_t* save, timerc* time) {
	CHUNK_t* chunk = FindChunk(save,TIMER_tag);
	chunk->pnt = 0;

	time->tstates	= ReadLong(chunk);
	time->freq		= (uint32_t) ReadLong(chunk);
	time->elapsed	= ReadDouble(chunk);
	time->lasttime	= ReadDouble(chunk);	// this isn't used.
}

void LoadLCD(SAVESTATE_t* save, LCD_t* lcd) {
	CHUNK_t* chunk = FindChunk(save,LCD_tag);
	chunk->pnt = 0;

	lcd->active		= ReadInt(chunk);
	lcd->word_len	= ReadInt(chunk);
	lcd->x			= ReadInt(chunk);
	lcd->y			= ReadInt(chunk);
	lcd->z			= ReadInt(chunk);
	lcd->cursor_mode		= (LCD_CURSOR_MODE) ReadInt(chunk);
	lcd->contrast	= ReadInt(chunk);
	lcd->base_level	= ReadInt(chunk);

	ReadBlock(chunk, lcd->display, DISPLAY_SIZE);
	lcd->front		= ReadInt(chunk);
	ReadBlock(chunk,  (unsigned char *) lcd->queue, LCD_MAX_SHADES * DISPLAY_SIZE);
	lcd->shades		= ReadInt(chunk);
	lcd->mode		= (LCD_MODE) ReadInt(chunk);
	lcd->time		= ReadDouble(chunk);
	lcd->ufps		= ReadDouble(chunk);
	lcd->ufps_last	= ReadDouble(chunk);
	lcd->lastgifframe= ReadDouble(chunk);
	lcd->write_avg	= ReadDouble(chunk);
	lcd->write_last = ReadDouble(chunk);
}



void LoadLINK(SAVESTATE_t* save, link_t* link) {
	CHUNK_t* chunk	= FindChunk(save,LINK_tag);
	chunk->pnt = 0;

	link->host		= ReadChar(chunk);
}

void LoadSTDINT(SAVESTATE_t* save, STDINT_t* stdint) {
	int i;
	CHUNK_t* chunk		= FindChunk(save,STDINT_tag);
	chunk->pnt = 0;

	stdint->intactive	= ReadChar(chunk);
	stdint->lastchk1	= ReadDouble(chunk);
	stdint->timermax1	= ReadDouble(chunk);
	stdint->lastchk2	= ReadDouble(chunk);
	stdint->timermax2	= ReadDouble(chunk);
	for(i = 0; i < 4; i++) {
		stdint->freq[i]	= ReadDouble(chunk);
	}
	stdint->mem			= ReadInt(chunk);
	stdint->xy			= ReadInt(chunk);
}

void LoadSE_AUX(SAVESTATE_t* save, SE_AUX_t *se_aux) {
	int i;
	if (!se_aux) {
		return;
	}
	CHUNK_t* chunk = FindChunk(save, SE_AUX_tag);
	if (!chunk) {
		return;
	}
	
	bool is_83p = save->model < TI_83PSE && save->version_minor == 1;
	if (is_83p) {
		LINKASSIST_t *linka = (LINKASSIST_t *) se_aux;
		linka->link_enable	= ReadChar(chunk);
		linka->in			= ReadChar(chunk);
		linka->out			= ReadChar(chunk);
		linka->working		= ReadChar(chunk);
		linka->receiving	= ReadInt(chunk);
		linka->read			= ReadInt(chunk);
		linka->ready		= ReadInt(chunk);
		linka->error		= ReadInt(chunk);
		linka->sending		= ReadInt(chunk);
		linka->last_access	= ReadDouble(chunk);
		linka->bit			= ReadInt(chunk);
		return;
	}
	
	se_aux->clock.enable		= ReadChar(chunk);
	se_aux->clock.set			= ReadInt(chunk);
	se_aux->clock.base			= ReadInt(chunk);
	se_aux->clock.lasttime		= ReadDouble(chunk);
	
	for(i = 0; i < 7; i++) {
		se_aux->delay.reg[i]	= ReadChar(chunk);
	}
	
	for(i = 0; i < NumElm(se_aux->md5.reg); i++)
	{
		se_aux->md5.reg[i]		= ReadInt(chunk);
	}
	se_aux->md5.s				= ReadChar(chunk);
	se_aux->md5.mode			= ReadChar(chunk);
	
	se_aux->linka.link_enable	= ReadChar(chunk);
	se_aux->linka.in			= ReadChar(chunk);
	se_aux->linka.out			= ReadChar(chunk);
	se_aux->linka.working		= ReadChar(chunk);
	se_aux->linka.receiving		= ReadInt(chunk);
	se_aux->linka.read			= ReadInt(chunk);
	se_aux->linka.ready			= ReadInt(chunk);
	se_aux->linka.error			= ReadInt(chunk);
	se_aux->linka.sending		= ReadInt(chunk);
	se_aux->linka.last_access	= ReadDouble(chunk);
	se_aux->linka.bit			= ReadInt(chunk);

	se_aux->xtal.lastTime		= ReadDouble(chunk);
	se_aux->xtal.ticks			= ReadLong(chunk);

	for(i = 0; i < 3; i++) {
		se_aux->xtal.timers[i].lastTstates	= ReadLong(chunk);
		se_aux->xtal.timers[i].lastTicks	= ReadDouble(chunk);
		se_aux->xtal.timers[i].divsor		= ReadDouble(chunk);
		se_aux->xtal.timers[i].loop			= ReadInt(chunk);
		se_aux->xtal.timers[i].interrupt	= ReadInt(chunk);
		se_aux->xtal.timers[i].underflow	= ReadInt(chunk);
		se_aux->xtal.timers[i].generate		= ReadInt(chunk);
		se_aux->xtal.timers[i].active		= ReadInt(chunk);
		se_aux->xtal.timers[i].clock		= ReadChar(chunk);
		se_aux->xtal.timers[i].count		= ReadChar(chunk);
		se_aux->xtal.timers[i].max			= ReadChar(chunk);
	}
	if (save->version_minor >= 1)
		se_aux->model_bits = ReadInt(chunk);
	else
		se_aux->model_bits = save->model == TI_84P ? 0 : 1;
	chunk = FindChunk(save, USB_tag);
	if (!chunk) return;
	chunk->pnt = 0;

	se_aux->usb.USBLineState = ReadInt(chunk);
	se_aux->usb.USBEvents = ReadInt(chunk);
	se_aux->usb.USBEventMask = ReadInt(chunk);
	se_aux->usb.LineInterrupt = ReadInt(chunk);
	se_aux->usb.ProtocolInterrupt = ReadInt(chunk);
	se_aux->usb.ProtocolInterruptEnabled = ReadInt(chunk);
	se_aux->usb.DevAddress = ReadInt(chunk);
	se_aux->usb.Port4A = ReadChar(chunk);
	se_aux->usb.Port4C = ReadChar(chunk);
	se_aux->usb.Port54 = ReadChar(chunk);
}


void LoadSlot(SAVESTATE_t *save, void *lpInput) {
	bool runsave;
	CALC* lpCalc = (CALC*) lpInput;
	
	if (lpCalc->active == false){
		puts("Slot was not active");
		return;
	}
	if (save == nullptr) {
		puts("Save was null");
		return;
	}
	
	runsave = lpCalc->running;
	lpCalc->running = false;
	
	LoadCPU(save, &lpCalc->cpu);
	LoadMEM(save, &lpCalc->mem_c);
	LoadTIMER(save, &lpCalc->timer_c);
	LoadLCD(save, lpCalc->cpu.pio.lcd);
	LoadLINK(save, lpCalc->cpu.pio.link);
	LoadSTDINT(save, lpCalc->cpu.pio.stdint);
	LoadSE_AUX(save, lpCalc->cpu.pio.se_aux);
	lpCalc->running = runsave;
}

char* GetRomOnly(SAVESTATE_t *save, int *size) {
	CHUNK_t* chunk = FindChunk(save, ROM_tag);
	*size = 0;
	if (!chunk) return nullptr;
	*size = chunk->size;
	return (char *) chunk->data;
}

const char * tmpSuffix = "/tmpsvXXXXXX";
	
void WriteSave(const char *fn, SAVESTATE_t* save, int compress) {
	int i;
	FILE* ofile;
	FILE* cfile;
	char temp_save[PATH_MAX];
	
	if (!save) {
		return;
	}
	if (compress == 0) {
		ofile = fopen(fn, "wb");
	} else {
		GetAppDataString(temp_save, PATH_MAX - 1 - strlen(tmpSuffix));
		strcat(temp_save, tmpSuffix);
		mkstemp(temp_save);
		ofile = fopen(temp_save,"wb");
	}
		
	if (!ofile) {
		return;
	}

	fputs(DETECT_STR, ofile);

	fputi(SAVE_HEADERSIZE, ofile);	
	
	fputi(save->version_major, ofile);
	fputi(save->version_minor, ofile);
	fputi(save->version_build, ofile);
	fputi(save->model, ofile);
	fputi(save->chunk_count, ofile);
	fwrite(save->author, 1,32, ofile);
	fwrite(save->comment, 1, 64, ofile);
	
	for(i = 0; i < save->chunk_count; i++) {
		fputc(save->chunks[i]->tag[0], ofile);
		fputc(save->chunks[i]->tag[1], ofile);
		fputc(save->chunks[i]->tag[2], ofile);
		fputc(save->chunks[i]->tag[3], ofile);
		fputi(save->chunks[i]->size,ofile);
		fwrite(save->chunks[i]->data, 1, save->chunks[i]->size, ofile);
	}
	fclose(ofile);
	
	if (compress) {
		cfile = fopen(fn, "wb");
		if (!cfile) {
			return;
		}
		ofile = fopen(temp_save,"rb");
		if (!ofile) {
			return;
		}
		//int error;
		fputs(DETECT_CMP_STR, cfile);
		switch(compress) {
#ifdef ZLIB_WINAPI
			case ZLIB_CMP:
				{
					fputc(ZLIB_CMP, cfile);
				
					int error = def(ofile, cfile, 9);
					break;
				}
#endif
			default:
				break;
		}
		fclose(ofile);
		fclose(cfile);
		remove(temp_save);
	}
}

SAVESTATE_t* ReadSave(FILE *ifile) {
	int i;
	int compressed = false;
	int chunk_offset,chunk_count;
	char string[128];
	char temp_save[PATH_MAX];
	SAVESTATE_t *save;
	CHUNK_t *chunk;
	FILE *tmpfile;

	fread(string, 1, 8, ifile);
	string[8] = 0;
	if (strncmp(DETECT_CMP_STR, string, 8) == 0) {
		i = fgetc(ifile);
		GetAppDataString(temp_save, PATH_MAX - 1 - strlen(tmpSuffix));
		strcat(temp_save, tmpSuffix);
		mkstemp(temp_save);
		tmpfile = fopen(temp_save,"wb");
		if (!tmpfile) {
			return nullptr;
		}
		//int error;
		switch(i) {
#ifdef ZLIB_WINAPI
			case ZLIB_CMP:
				{
					int error = inf(ifile,tmpfile);
					fclose(tmpfile);
					ifile = fopen(temp_save,"rb");	//this is not a leak, file gets closed
													// outside of this routine.
					if (!ifile) {
						return nullptr;
					}
					compressed = true;
					fread(string, 1, 8, ifile);
					break;
				}
#endif
			default:
				fclose(tmpfile);
				remove(temp_save);
				return nullptr;
		}
	}
		
	if (strncmp(DETECT_STR, string, 8) != 0){

		if (compressed == true) fclose(ifile);
		return nullptr;
	}		
	
	save = (SAVESTATE_t *) malloc(sizeof(SAVESTATE_t));
	if (!save) {
		if (compressed == true) fclose(ifile);
		return nullptr;
	}

	chunk_offset = fgeti(ifile);
	
	save->version_major = fgeti(ifile);
	save->version_minor = fgeti(ifile);
	save->version_build = fgeti(ifile);


	if (save->version_major != CUR_MAJOR) {
		fclose(ifile);
		free(save);
		return nullptr;
	}

	save->model = fgeti(ifile);

	chunk_count = fgeti(ifile);
	fread(save->author,1,32,ifile);
	fread(save->comment,1,64,ifile);

	fseek(ifile, chunk_offset + 8 + 4, SEEK_SET);
	
	for(i = 0; i < 512; i++) {
		save->chunks[i] = nullptr;
	}
	save->chunk_count = 0;
	for(i = 0; i < chunk_count; i++) {
		string[0]	= fgetc(ifile);
		string[1]	= fgetc(ifile);
		string[2]	= fgetc(ifile);
		string[3]	= fgetc(ifile);
		string[4]	= 0;
		chunk		= NewChunk(save,string);
		chunk->size	= fgeti(ifile);
		chunk->data	= (unsigned char *) malloc(chunk->size);
		fread(chunk->data,1,chunk->size,ifile);
	}
	if (compressed == true) {
		fclose(ifile);
		remove(temp_save);
	}
/* check for read errors... */
	return save;
}
