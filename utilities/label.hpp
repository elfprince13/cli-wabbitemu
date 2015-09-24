#ifndef _LABEL_H_TYPES
#define _LABEL_H_TYPES
#include "../core/coretypes.hpp"

typedef struct {
    char *name;
    bool IsRAM;
    uint8_t page;
    uint16_t addr;
} label_struct;

#endif

#ifndef _LABEL_H_PROTOTYPES

#include "../interface/calc.hpp"

#ifdef _HAS_CALC_H
#define _LABEL_H_PROTOTYPES
char* FindAddressLabel(CALC* lpCalc, waddr_t waddr);
//void ImportBcalls(char* fn);
char* FindBcall(int address);
void FindFlags(int flag, int bit, char **flagstring, char **bitstring);

void VoidLabels(CALC* lpCalc);
label_struct *lookup_label(CALC* lpCalc, char *label);
int labels_app_load(CALC* lpCalc, const char * lpszFileName);
#endif

#endif

