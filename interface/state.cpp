
#include "state.hpp"
#include "../hardware/link.hpp"

#ifndef WINVER
#define min(a, b) ((a) > (b) ? (b) : (a))
#endif

void state_userpages(CPU_t *, upages_t*);
char *symbol_to_string(CPU_t *, symbol83P_t *, char *);

int find_header(uint8_t (*dest)[PAGE_SIZE], int page, int ident1, int ident2) {
	int i;
	//apparently non user apps have a slightly different header
	//therefore we have to actually find the identifier
	for (i = 0; i < PAGE_SIZE; i++)
		if (dest[page][i] == ident1 && dest[page][i + 1] == ident2)
			return dest[page][i + 2];
	return -1;	
}

/* Generate a list of applications */
void state_build_applist(CPU_t *cpu, applist_t *applist) {
	applist->count = 0;
	
	if (cpu->mem_c == nullptr)
	{
		return;
	}

	if (cpu->mem_c->flash == nullptr) return;
	uint8_t (*flash)[PAGE_SIZE] = (uint8_t (*)[PAGE_SIZE]) cpu->mem_c->flash;

	// fetch the userpages for this model
	upages_t upages;
	state_userpages(cpu, &upages);
	if (upages.start == -1) return;
	
	// Starting at the first userpage, search for all the apps
	// As soon as page doesn't have one, you're done
	uint32_t page, page_size;
	for (	page = upages.start, applist->count = 0;
			page >= upages.end &&
			applist->count < ARRAYSIZE(applist->apps) &&
			flash[page][0x00] == 0x80 && flash[page][0x01] == 0x0F &&
			find_header(flash, page, 0x80, 0x48) != -1 &&
			(page_size = find_header(flash, page, 0x80, 0x81)) != -1;
			
			page -= page_size, applist->count++) {
		
		apphdr_t *ah = &applist->apps[applist->count];
		uint32_t i;
		for (i = 0; i < PAGE_SIZE; i++)
			if (flash[page][i] == 0x80 && flash[page][i + 1] == 0x48)
				break;
		memcpy(ah->name, &flash[page][i + 2], 8);
		ah->name[8] = '\0';
		ah->page = page;
		ah->page_count = find_header(flash, page, 0x80, 0x81);

	}
}

symlist_t *state_build_symlist_86(CPU_t *cpu, symlist_t *symlist) {
	if (cpu->pio.model != TI_86) {
		return nullptr;
	}

	memc *mem = cpu->mem_c;

	uint16_t end = mem_read16(mem, VAT_END_86) & 0x3FFF;
	waddr_t stp;
	stp.addr = 0x3FFF;
	stp.is_ram = true;
	stp.page = 7;


	symbol83P_t *sym;
	// Loop through while stp is still in the symbol table
	for (sym = symlist->symbols; stp.addr > end; sym++) {
		sym->type_ID		= wmem_read(mem, stp) & 0x1F;
		stp.addr--;
		sym->address		= wmem_read(mem, stp);
		stp.addr--;
		sym->address		+= (wmem_read(mem, stp) << 8);
		stp.addr--;
		sym->page			= wmem_read(mem, stp);
		stp.addr--;
		if (sym->page > 0) {
			int total = (sym->page << 16) + sym->address;
			sym->page = (total - 0x10000) / PAGE_SIZE + 1;
			sym->address %= PAGE_SIZE;
		}
		sym->type_ID2		= wmem_read(mem, stp);
		stp.addr--;
		
		int i;
		sym->name_len = wmem_read(mem, stp);
		stp.addr--;
		for (i = 0; i < sym->name_len; i++) {
			sym->name[i] = wmem_read(mem, stp);
			stp.addr--;
		}
		sym->name[i] = '\0';
		symlist->last = sym;
	}
	
	return symlist;
}

symlist_t* state_build_symlist_83P(CPU_t *cpu, symlist_t *symlist) {
	memc *mem = cpu->mem_c;
	// end marks the end of the symbol table
	uint16_t 	end = mem_read16(mem, pTemp),
	// prog denotes where programs start
	prog = mem_read16(mem, progPtr),
	// stp (symbol table pointer) marks the start
	stp = symTable;
	
	// Verify VAT integrity
	if (cpu->pio.model < TI_83P) return nullptr;
	if (stp < end || stp < prog) return nullptr;
	if (end > prog || end < 0x9D95) return nullptr;
	if (prog < 0x9D95) return nullptr;

	symbol83P_t *sym;
	// Loop through while stp is still in the symbol table
	for (sym = symlist->symbols; stp > end; sym++) {
		
		sym->type_ID		= mem_read(mem, stp--) & 0x1F;
		sym->type_ID2		= mem_read(mem, stp--);
		sym->version		= mem_read(mem, stp--);
		sym->address		= mem_read(mem, stp--);
		sym->address		+= (mem_read(mem, stp--) << 8);
		sym->page			= mem_read(mem, stp--);
		sym->length			= mem_read(mem, sym->address - 1) + (mem_read(mem, sym->address) << 8);
		
		uint32_t i;
		// Variables, pics, etc
		if (stp > prog) {
			for (i = 0; i < 3; i++) sym->name[i] = mem_read(mem, stp--);
			symlist->programs = sym + 1;
		// Programs
		} else {
			sym->name_len = mem_read(mem, stp--);
			for (i = 0; i < sym->name_len; i++) sym->name[i] = mem_read(mem, stp--);
			sym->name[i] = '\0';
			symlist->last = sym;
		}
	}
	
	return symlist;
}

/* Find a symbol in a symlist.
 * Provide a name and name length and the symbol, if found is returned.
 * Otherwise, return nullptr */
symbol83P_t *search_symlist(symlist_t *symlist, const char *name, size_t name_len) {
	symbol83P_t *sym = symlist->symbols;
	while (sym <= symlist->last && memcmp(sym->name, name, name_len)) sym++;
	
	if (sym > symlist->last) return nullptr;
	return sym;
}

char *App_Name_to_String(apphdr_t *app, char *buffer) {
#ifdef WINVER
	StringCbCopy(buffer, strlen(app->name) + 1, app->name);
	return buffer;
#else
	return strcpy(buffer, app->name);
#endif
}


#ifdef WINVER
char *Symbol_Name_to_String(int model, symbol83P_t *sym, char *buffer) {
	const char ans_name[] = {tAns, 0x00, 0x00};
	if (memcmp(sym->name, ans_name, 3) == 0) {
		StringCbCopy(buffer, 10, ("Ans"));
		return buffer;
	}
	
	if (model == TI_86) {
		StringCbCopy(buffer, 10, sym->name);
		return buffer;
	} else {
		switch(sym->type_ID) {
			case ProgObj:
			case ProtProgObj:
			case AppVarObj:
			case GroupObj: {
				errno_t error = StringCbCopy(buffer, 10, sym->name);
				return buffer;
			}
			case PictObj:
				StringCbPrintf(buffer, 10, ("Pic%d"), circ10(sym->name[1]));
				return buffer;
			case GDBObj:
				StringCbPrintf(buffer, 10, ("GDB%d"), circ10(sym->name[1]));
				return buffer;
			case StrngObj:
				StringCbPrintf(buffer, 10, ("Str%d"), circ10(sym->name[1]));
				return buffer;		
			case RealObj:
			case CplxObj:
				StringCbPrintf(buffer, 10, ("%c"), sym->name[0]);
				return buffer;
			case ListObj:
			case CListObj:
				if ((uint8_t) sym->name[1] < 6) {
					StringCbPrintf(buffer, 10, ("L%d"), sym->name[1] + 1); //L1...L6
				} else {
					StringCbPrintf(buffer, 10, ("%s"), sym->name + 1); // No Little L
				}
				return buffer;
			case MatObj:
				if (sym->name[0] == 0x5C) {
					StringCbPrintf(buffer, 10, ("[%c]"), 'A' + sym->name[1]);
					return buffer;
				}
				return nullptr;
		//	case EquObj:
		//	case NewEquObj:
		//	case UnknownEquObj:
			case EquObj_2:
				{
					if (sym->name[0] != 0x5E) {
						return nullptr;
					}
			
					uint8_t b = sym->name[1] & 0x0F;
					switch(sym->name[1] & 0xF0) {
						case 0x10: //Y1
							StringCbPrintf(buffer, 10, ("Y%d"),circ10(b));
							return buffer;
						case 0x20: //X1t Y1t
							StringCbPrintf(buffer, 10, ("X%dT"), ((b/2)+1)%6);
							if (b % 2) {
								buffer[0] = 'Y';
							}
							return buffer;
						case 0x40: //r1
							StringCbPrintf(buffer, 10, ("R%d"),(b+1)%6);
							return buffer;
						case 0x80: //Y1
							switch (b) {
								case 0: 
									StringCbCopy(buffer, 10, ("Un"));
									return buffer;
								case 1: 
									StringCbCopy(buffer, 10, ("Vn"));
									return buffer;
								case 2: 
									StringCbCopy(buffer, 10, ("Wn"));
									return buffer;
							}
						default: 
							return nullptr;
					}
				}
			default:
				return nullptr;
		}
	}
}
#else
char *Symbol_Name_to_String(int model, symbol83P_t *sym, char *buffer) {
	const char ans_name[] = {tAns, 0x00, 0x00};
	if (memcmp(sym->name, ans_name, 3) == 0) {
		strcpy(buffer, ("Ans"));
		return buffer;
	}
	
	if (model == TI_86) {
		strcpy(buffer, sym->name);
		return buffer;
	} else {
		switch(sym->type_ID) {
			case ProgObj:
			case ProtProgObj:
			case AppVarObj:
			case GroupObj: {
				strcpy(buffer, sym->name);
				return buffer;
			}
			case PictObj:
				sprintf(buffer, ("Pic%d"), circ10(sym->name[1]));
				return buffer;
			case GDBObj:
				sprintf(buffer, ("GDB%d"), circ10(sym->name[1]));
				return buffer;
			case StrngObj:
				sprintf(buffer, ("Str%d"), circ10(sym->name[1]));
				return buffer;		
			case RealObj:
			case CplxObj:
				sprintf(buffer, ("%c"), sym->name[0]);
				return buffer;
			case ListObj:
			case CListObj:
				if ((uint8_t) sym->name[1] < 6) {
					sprintf(buffer, ("L%d"), sym->name[1] + 1); //L1...L6
				} else {
					sprintf(buffer, ("%s"), sym->name + 1); // No Little L
				}
				return buffer;
			case MatObj:
				if (sym->name[0] == 0x5C) {
					sprintf(buffer, ("[%c]"), 'A' + sym->name[1]);
					return buffer;
				}
				return nullptr;
		//	case EquObj:
		//	case NewEquObj:
		//	case UnknownEquObj:
			case EquObj_2:
				{
					if (sym->name[0] != 0x5E) {
						return nullptr;
					}
			
					uint8_t b = sym->name[1] & 0x0F;
					switch(sym->name[1] & 0xF0) {
						case 0x10: //Y1
							sprintf(buffer, ("Y%d"),circ10(b));
							return buffer;
						case 0x20: //X1t Y1t
							sprintf(buffer, ("X%dT"), ((b/2)+1)%6);
							if (b % 2) {
								buffer[0] = 'Y';
							}
							return buffer;
						case 0x40: //r1
							sprintf(buffer, ("R%d"), (b+1)%6);
							return buffer;
						case 0x80: //Y1
							switch (b) {
								case 0: 
									strcpy(buffer, ("Un"));
									return buffer;
								case 1: 
									strcpy(buffer, ("Vn"));
									return buffer;
								case 2: 
									strcpy(buffer, ("Wn"));
									return buffer;
							}
						default: 
							return nullptr;
					}
				}
			default:
				return nullptr;
		}
	}
}
#endif

void SetRealAns(CPU_t *cpu, char *text) {
	//here is our general strategy:
	//1. save appbackupscreen
	//2. 
	int i;
	unsigned char appbackupscreen[768];
	for (i = 0; i < 768; i++)
		appbackupscreen[i] = mem_read(cpu->mem_c, 0x9872 + i);

}

char *GetRealAns(CPU_t *cpu) {
	symlist_t symlist;
	state_build_symlist_83P(cpu, &symlist);
	
	const char ans_name[] = {tAns, 0x00, 0x00};
	symbol83P_t *sym = search_symlist(&symlist, ans_name, 3);
	if (sym == nullptr) return nullptr;

#ifdef WINVER
	char *buffer = (char *) LocalAlloc(LMEM_FIXED, 2048);
#else
	char *buffer = (char *) malloc(2048);
#endif
	
	symbol_to_string(cpu, sym, buffer);
	
	return buffer;
}

/* Print a symbol's value to a string */
char *symbol_to_string(CPU_t *cpu, symbol83P_t *sym, char *buffer) {
	switch (sym->type_ID) {
	case CplxObj:
	case RealObj: {
		uint16_t ptr = sym->address;
		char *p = buffer;
		bool is_imaginary = false;
	TI_num_extract:
		;
		uint8_t type = mem_read(cpu->mem_c, ptr++);
		int exp = (char) (mem_read(cpu->mem_c, ptr++) ^ 0x80);
		if (exp == -128) exp = 128;
		
		uint8_t FP[14];
		int i, sigdigs = 1;
		for (i = 0; i < 14; i+=2, ptr++) {
			FP[i] 	= mem_read(cpu->mem_c, ptr) >> 4;
			FP[i+1]	= mem_read(cpu->mem_c, ptr) & 0x0F;
			if (FP[i]) sigdigs = i + 1;
			if (FP[i+1]) sigdigs = i + 2;
		}
		
		if (type & 0x80) *p++ = '-';
		else if (is_imaginary) *p++ = '+';
		
		if (abs(exp) > 14) {
			for (i = 0; i < sigdigs; i++) {
				*p++ = FP[i] + '0';
				if ((i + 1) < sigdigs && i == 0) *p++ = '.';
			}
#ifdef WINVER
			StringCbPrintf(p, strlen(p), ("*10^%d"), exp);
#else
			sprintf(p, ("*10^%d"), exp);
#endif
			p += strlen(p);
		} else {
			for (i = min(exp, 0); i < sigdigs || i < exp; i++) {
				*p++ = (i >= 0 ? FP[i] : 0) + '0';
				if ((i + 1) < sigdigs && i == exp) *p++ = '.';
			}
		}
		
		if (is_imaginary) {
			*p++ = 'i';
		} else {
			// Is this real part of a complex number?
			if ((type & 0x0F) == 0x0C) {
				is_imaginary = true;
				goto TI_num_extract;
			}
		}
		*p++ = '\0';
		
		return buffer;
	}
	case CListObj:
	case ListObj: {
		symbol83P_t elem;
		
		// A false symbol for each element of the array
		elem.type_ID 	= sym->type_ID - 1;
		elem.page 		= 0;
		
		uint32_t i, size = mem_read16(cpu->mem_c, sym->address);
		char *p = buffer;
		*p++ = '{';
		
		uint16_t ptr;
		for (i = 0, ptr = sym->address + 2; i < size; i++) {
			if (i != 0) *p++ = ',';
			elem.address = ptr;
			symbol_to_string(cpu, &elem, p);
			p += strlen(p);
			ptr += (elem.type_ID == RealObj) ? 9 : 18;
		}
		
		*p++ = '}';
		*p++ = '\0';
		return buffer;
	}
	case MatObj: {
		symbol83P_t elem;
		
		elem.type_ID 	= RealObj;
		elem.page 		= 0;
		
		uint32_t cols = mem_read(cpu->mem_c, sym->address);
		uint32_t rows = mem_read(cpu->mem_c, sym->address + 1);
		uint32_t i, j;
		char *p = buffer;
		
		*p++ = '[';
		uint16_t ptr = sym->address + 2;
		for (j = 0; j < rows; j++) {
			if (j != 0) *p++ = ' ';
			
			*p++ = '[';
			for (i = 0; i < cols; i++, ptr += 9) {
				if (i != 0) *p++ = ',';
				elem.address = ptr;
				symbol_to_string(cpu, &elem, p);
				p += strlen(p);
			}
			*p++ = ']';
			if (j + 1 < rows) {
				*p++ = '\r';
				*p++ = '\n';
			}
		}
		*p++ = ']';
		*p++ = '\0';
		return buffer;
	}
		
		
	case StrngObj: {
		// There's a problem here with character set conversions
		uint16_t ptr = sym->address;
		uint16_t str_len = mem_read(cpu->mem_c, ptr++) + (mem_read(cpu->mem_c, ptr++) << 8);
		uint32_t i;
		for (i = 0; i < str_len; i++) buffer[i] = mem_read(cpu->mem_c, ptr++);
		buffer[i] = '\0';
		return buffer;
	}
	
		
	default:
#ifdef WINVER
		StringCbCopy(buffer, strlen(buffer), ("unsupported"));
#else
		strcpy(buffer, ("unsupported"));
#endif
		return buffer;
	}
}

void state_userpages(CPU_t *cpu, upages_t *upages) {
	switch (cpu->pio.model) {
		case TI_73:
			upages->start	= TI_73_APPPAGE;
			upages->end		= upages->start - TI_73_USERPAGES;
			break;
		case TI_83P:
			upages->start	= TI_83P_APPPAGE;
			upages->end		= upages->start - TI_83P_USERPAGES;
			break;
		case TI_83PSE:
			upages->start	= TI_83PSE_APPPAGE;
			upages->end		= upages->start - TI_83PSE_USERPAGES;
			break;
		case TI_84P:
			upages->start	= TI_84P_APPPAGE;
			upages->end		= upages->start - TI_84P_USERPAGES;
			break;
		case TI_84PSE:
			upages->start	= TI_84PSE_APPPAGE;
			upages->end		= upages->start - TI_84PSE_USERPAGES;
			break;
		default:
			upages->start	= -1;
			upages->end		= 0;
			break;
	}	
}
