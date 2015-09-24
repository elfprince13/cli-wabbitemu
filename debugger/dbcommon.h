#ifndef DBCOMMON_H
#define DBCOMMON_H

#include "../interface/calc.hpp"

typedef enum {
	HEX2,
	HEX4,
	FLOAT2,
	FLOAT4,
	DEC3,
	DEC5,
	BIN8,
	BIN16,
	CHAR1,
} VALUE_FORMAT;

typedef enum {
	HEX,
	DEC,
	BIN,
} DISPLAY_BASE;

typedef enum {
	REGULAR,			//view paged memory
	FLASH,				//view all flash pages
	RAM,				//view all ram pages
} ViewType;

typedef struct {
	int total;
	bool state[32];
} ep_state;

#endif /* DBCOMMON_H */
