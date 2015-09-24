#define LABEL_C
#include "label.hpp"
#include "../core/core.hpp"
#include "../interface/calc.hpp"

#include "bcalls.hpp"
#include "flags.hpp"

label_struct *lookup_label(CALC* lpCalc, char *label_name) {
	int i;
	for (i = 0; lpCalc->labels[i].name != nullptr; i++) {
		if (strcasecmp(lpCalc->labels[i].name, label_name) == 0)
			return &lpCalc->labels[i];
	}
	return nullptr;
}	
	

void VoidLabels(CALC* lpCalc) {
	int i;
	
	for (i = 0; lpCalc->labels[i].name != nullptr; i++) {
		free(lpCalc->labels[i].name);
		lpCalc->labels[i].name = nullptr;
	}
}

char* FindAddressLabel(CALC* lpCalc, waddr_t waddr) {
	
	for (int i = 0; lpCalc->labels[i].name != nullptr; i++) {
		label_struct *label = &lpCalc->labels[i];
		if (label->IsRAM == waddr.is_ram && label->page == waddr.page && label->addr == waddr.addr)
			return label->name;
	}
	return nullptr;
}
	
//-------------------------------------------
// True means label is found and is the same
//
bool label_search_tios(char *label, int equate) {
	if (!label) {
		return false;
	}

	for(int i = 0; bcalls[i].address != -1; i++ ) {
		if (strcasecmp(label, bcalls[i].name) == 0) {
			if (bcalls[i].address == (equate & 0xFFFF) ) {
				return true;
			}
		}
	}
	
	for(int i = 0; flags83p[i].flag != -1; i++ ) {
		if (strcasecmp(label, flags83p[i].name) == 0) {
			if (flags83p[i].flag == (equate & 0xFFFF)) {
				return true;
			}
		}
		for(int b = 0; b < 8; b++) {
			if (strcasecmp(label, flags83p[i].bits[b].name) == 0) {
				if (flags83p[i].bits[b].bit == (equate & 0xFFFF)) {
					return true;
				}
			}
		}
	}
	return false;
}
	

int labels_app_load(CALC* lpCalc, const char * lpszFileName) {
	FILE *labelFile = nullptr;
	int i, length;
#ifdef _UNICODE
	char readBuf[256];
#endif
	char buffer[256];
	char name[256];
	char *fileName = ((char *) lpszFileName) + strlen(lpszFileName);
	while (*--fileName != '\\');
	fileName++;

	unsigned int equate;
	label_struct *label = &lpCalc->labels[0];	

#ifdef _WINDOWS
	fopen(&labelFile, lpszFileName, ("r"));
#else
	labelFile = fopen(lpszFileName, "r");
#endif
	if (labelFile == nullptr) {
		return 1;
	}
	
	// Clear out the old labels
	VoidLabels(lpCalc);

	while (!feof(labelFile)) {
#ifdef _UNICODE
#ifdef _WINDOWS
		fgets(readBuf, 256, labelFile);
		MultiByteToWideChar(CP_ACP, 0, readBuf, -1, buffer, ARRAYSIZE(buffer));
#else
		fgets(readBuf, 256, labelFile);
		
#endif
#else
		fgets(buffer, 256, labelFile);
#endif
		i = 0;
		if (buffer[0] != ';')
			i = sscanf(buffer, ("%s = $%X"), name, &equate);
		if (i == 2) {
			length = (int) strlen(name);
			if (!label_search_tios(name, equate)) {
				label->name = (char *) malloc((length + 1) * sizeof(char));
#ifdef _WINDOWS
				StringCchCopy(label->name, length + 1, name);
#else
				strcpy(label->name, name);
#endif

				label->addr = equate & 0xFFFF;

				if ( (equate & 0x0000FFFF) >= 0x4000 && (equate & 0x0000FFFF) < 0x8000) {					
					label->IsRAM = false;
					if (lpCalc->last_transferred_app == nullptr) {
						upages_t upage;
						state_userpages(&lpCalc->cpu, &upage);
						label->page = upage.start;
					} else {
						applist_t applist;
						state_build_applist(&lpCalc->cpu, &applist);
						for (int i = 0; i < applist.count; i++) {
							int len = 8;
							char *ptr = applist.apps[i].name + len - 1;
							while (isspace(*ptr--))
								len--;
							if (!strncasecmp(fileName, applist.apps[i].name, len)) {
								label->page = applist.apps[i].page;
								break;
							}
						}
					}
				} else {
					label->IsRAM = true;
					label->page = 1;
				}
				label++;
			}
		}
	}
	fclose(labelFile);
	return 0;
}
	
/*
void ImportBcalls(char* fn) {
	int i,address;
	char string[256],tmp[32];
	FILE* infile;
	
	infile = fopen(fn,"r");
	
	if (!infile) {
		puts("COuld not open bcall file");
		return;
	}
	for(address=0;address<65536;address++) {
		for(i=0;i<32;i++) bcalls[address][i] = 0;
	}		
	while( !feof(infile) ) {
		fgets(string,256,infile);
		i = sscanf(string,"%s = $%04X",tmp,&address);
		if (i == 2) {
			strcpy(bcalls[address],tmp);
		}
	}
	fclose(infile);
}
*/
char* FindBcall(int address) {
	for(int i = 0; bcalls[i].address != -1; i++ ) {
		if (bcalls[i].address == address) {
			return bcalls[i].name;
		}
	}
	return nullptr;
}


void FindFlags(int flag, int bit, char **flagstring, char **bitstring) {
	int i,b;
	for(i = 0; flags83p[i].flag != -1; i++ ) {
		if (flags83p[i].flag == flag) {
			for(b = 0; b < 8; b++) {
				if (flags83p[i].bits[b].bit == bit) {
					*flagstring = flags83p[i].name;
					*bitstring  = flags83p[i].bits[b].name;
					return;
				}
			}
		}
	}
	*flagstring = nullptr;
	*bitstring  = nullptr;
}
	
	
	
	
	

