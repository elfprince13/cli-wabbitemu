#ifndef STATE_H_
#define STATE_H_

#include "../core/core.hpp"
#include "../utilities/var.hpp"		// TIFILE

typedef struct apphdr {
	char name[12];
	uint32_t page, page_count;
} apphdr_t;

typedef struct applist {
	uint32_t count;
	apphdr_t apps[96];
} applist_t;

typedef struct {
	uint8_t type_ID;
	uint8_t type_ID2;
	uint8_t version;
	uint16_t address;
	uint8_t page;
	uint8_t name_len;
	uint8_t length;
	char name[9];
} symbol83P_t;

typedef struct symlist {
	symbol83P_t *programs;
	symbol83P_t *last;
	symbol83P_t symbols[512];
} symlist_t;

//83p
#define pTemp			0x982E
#define progPtr			0x9830
#define symTable		0xFE66
//86
#define VAT_END_86		0xD298

typedef struct upages {
	uint32_t start, end;
} upages_t;

#define circ10(z) ((((uint8_t) z) < 10) ? ((z) + 1) % 10 : (z))
#define tAns	0x72

void state_build_applist(CPU_t *, applist_t *);
void state_userpages(CPU_t *, upages_t *);
symlist_t *state_build_symlist_86(CPU_t *, symlist_t *);
symlist_t *state_build_symlist_83P(CPU_t *, symlist_t *);
char *GetRealAns(CPU_t*);
char *Symbol_Name_to_String(int model, symbol83P_t *, char *);
char *App_Name_to_String(apphdr_t *, char *);

#endif /*STATE_H_*/
