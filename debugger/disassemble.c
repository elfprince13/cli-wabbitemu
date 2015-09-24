#include "disassemble.h"
#include "label.h"
#include "calc.h"

//http://www.z80.info/decoding.htm

// %r - register
// %g - relative address
// %a - 2 byte address
// %x - 1 hex string
// %s - regular string
// %d - decimal number
// %h - signed offset
// %c - condition
// %l - label string
Z80_com_t da_opcode[256] = {
{("  nop"), 				4, 	0},
{("  ex %r,%r"),			4, 	0},
{("  djnz %g"),			13,	8},
{("  jr %g"),				12, 0},
{("  jr %c,%g"),			12,	7},
{("  ld %r,%x"),			10, 0},
{("  add %r,%r"),			11, 0},
{("  ld (%r),%r"),		7,	0},
{("  ld %r,(%r)"),		7,	0},
{("  ld (%r),%r"),		7,	0},
{("  ld %r,(%r)"),		7,	0},
{("  ld (%a),%r"),		16,	0},
{("  ld %r,(%a)"),		16,	0},
{("  ld (%a),%r"),		13,	0},
{("  ld %r,(%a)"),		13,	0},
{("  inc %r"),			6,	0},
{("  dec %r"),			6,	0},
{("  inc %r"),			4,	0},
{("  dec %r"),			4,	0},
{("  ld %r,%x"),			7,	0},
{("  rlca"),				4,	0},
{("  rrca"),				4,	0},
{("  rla"),				4,	0},
{("  rra"),				4,	0},
{("  daa"),				4,	0},
{("  cpl"),				4,	0},
{("  scf"),				4,	0},
{("  ccf"),				4,	0},
{("  ld %r,%r"),			4,	0},
{("  halt"),				4,	0},
{("  %s %r"),			4,	0},
{("  %s %r,%r"),			4,	0},
{("  ret %c"),		11,	5},
{("  pop %r"),		10,	0},
{("  ret"),			10, 0},
{("  exx"),			4,	0},
{("  jp (%r)"),		4,	0},
{("  ld %r,%r"),		6,	0},
{("  jp %c,%a"),		10,	0},
{("  jp %a"),			10,	0},
{("  out (%x),%r"),	11,	0},
{("  in %r,(%x)"),		11, 0},
{("  ex (%r),%r"),	19, 0},
{("  ex %r,%r"),		4,	0},
{("  di"),			4,	0},
{("  ei"),			4,	0},
{("  call %c,%a"),	17,	10},
{("  push %r"),		11,	0},
{("  call %a"),		17,	0},
{("  %s %d"),			4,	0},
{("  %s %r,%d"),		4,	0},
{("  rst %xh"),		11,	0},
{("  %s %s"),			-1,	0},
{("  bit %d,%r"),		8,	0},
{("  res %d,%r"),		8,	0},
{("  set %d,%r"),		8,	0},
{("  in %r,(%r)"),	12,	0},
{("  out (%r),%r"),	12,	0},
{("  sbc %r,%r"),		15,	0},
{("  adc %r,%r"),		15,	0},
{("  ld (%a),%r"),	20,	0},
{("  ld %r,(%a)"),	20,	0},
{("  neg"),			8,	0},
{("  retn"),			14,	0},
{("  reti"),			14,	0},
{("  im %s"),			8,	0},
{("  ld i,a"),		9,	0},
{("  ld r,a"),		9,	0},
{("  ld a,i"),		9,	0},
{("  ld a,r"),		9,	0},
{("  rrd"),			18,	0},
{("  rld"),			18, 0},
{("  nop"),			8,	0},
{("  %s"),			21,	16},
{("  %s (%r%h)->%r"),	23,	0},
{("  bit %d,(%r%h)->%r"),	23,	0},
{("  res %d,(%r%h)->%r"),	23,	0},
{("  set %d,(%r%h)->%r"),	23,	0},
{("  %s (%s%h)"),		23,	0},
{("  bit %d,(%r%h)"),	20,	0},
{("  res %d,(%r%h)"),	20,	0},
{("  set %d,(%r%h)"),	20,	0},
{("  add %r,%r"),		15,	0},
{("  ld %r,(%a)"),	20,	0},
{("  ld (%a),%r"),	20,	0},
{("  inc (%r%h)"),	23,	0},
{("  dec (%r%h)"),	23,	0},
{("  ld (%r%h),%x"),	19,	0},
{("  ld (%r%h),%r"),	19,	0},
{("  ld %r,(%r%h)"),	19,	0},
{("  %s (%r%h)"),		19,	0},
{("  %s %r,(%r%h)"),	19, 0},
{("  jp %s"),			6,	0},
{("  ld %r,%r"),		10, 0},
{("  ex (sp),%s"),	23,	0},
{("%l:"),				-1, 0},
{("  bcall(%l)"),		-1,	0},
{("  bcall(%a)"),		-1,	0},
{("  bit %l,(%r+%l)->%r"),	23,	0},
{("  res %l,(%r+%l)->%r"),	23,	0},
{("  set %l,(%r+%l)->%r"),	23,	0},
{("  bit %l,(%r+%l)"),	20,	0},
{("  res %l,(%r+%l)"),	20,	0},
{("  set %l,(%r+%l)"),	20,	0},
{("  bjump(%l)"),			-1,	0},
{("  bjump(%a)"),			-1,	0},
{("  ld %r,%x"),			10,	0},
{("  ld %r,%r"),			7,	0},
{("  inc %r"),			11,	0},
{("  dec %r"),			11,	0},
{("  bit %d,%r"),			12,	0},
{("  res %d,%r"),			15,	0},
{("  set %d,%r"),			15,	0},
};

#define R_B 0
#define R_C 1
#define R_D 2
#define R_E 3
#define R_H 4
#define R_L 5
#define R__HL_ 6
#define R_A 7
#define R_F 8
static char r[9][5]		= {("b"),("c"),("d"),("e"),("h"),("l"),("(hl)"),("a"),("f")};
static char r8i[2][9][5]	= {
{("b"),("c"),("d"),("e"),("ixh"),("ixl"),("ix"),("a"),("f")},
{("b"),("c"),("d"),("e"),("iyh"),("iyl"),("iy"),("a"),("f")}};
#define R_BC 0
#define R_DE 1
#define R_HL 2
#define R_SP 3
#define R_AF 3
static char rp[4][4]		= {("bc"),("de"),("hl"),("sp")};
static char rpi[2][4][4]	= {{("bc"),("de"),("ix"),("sp")},{("bc"),("de"),("iy"),("sp")}};
static char rp2[4][4]		= {("bc"),("de"),("hl"),("af")};
static char rp2i[2][4][4]	= {{("bc"),("de"),("ix"),("af")},{("bc"),("de"),("iy"),("af")}};
static char ri[2][4]		= {("ix"),("iy")};
static char cc[8][4]		= {("nz"),("z"),("nc"),("c"),("po"),("pe"),("p"),("m")};
static char alu[8][4]		= {("add"),("adc"),("sub"),("sbc"),("and"),("xor"),("or"),("cp")};
static char rot[8][4]		= {("rlc"),("rrc"),("rl"),("rr"),("sla"),("sra"),("sll"),("srl")};
static char im[8][4]		= {("0"),("0/1"),("1"),("2"),("0"),("0/1"),("1"),("2")};
static char bli[4][4][8]	= {
{("ldi"), ("cpi"), ("ini"), ("outi")},
{("ldd"), ("cpd"), ("ind"), ("outd")},
{("ldir"),("cpir"),("inir"),("otir")},
{("lddr"),("cpdr"),("indr"),("otdr")}};

waddr_t GetNextAddr(memory_context_t *memc, ViewType type, waddr_t waddr) {
	switch (type) {
		case REGULAR:
			waddr = addr_to_waddr(memc, waddr.addr + 1);
			break;
		case RAM:
		case FLASH:
			waddr.addr = (waddr.addr + 1) % PAGE_SIZE;
			if (!(waddr.addr % PAGE_SIZE))
				waddr.page++;
			int pages = type == FLASH ? memc->flash_pages : memc->ram_pages;
			waddr.page %= pages;
			break;
	}
	return waddr;
}

waddr_t OffsetWaddr(memory_context_t *memc, ViewType type, waddr_t waddr, int offset) {
	switch (type) {
		case REGULAR:
			waddr = addr_to_waddr(memc, waddr.addr + offset);
			break;
		case RAM:
		case FLASH: {
			int old_abs_addr = waddr.addr;
			waddr.addr = (waddr.addr + offset) % PAGE_SIZE;
			int new_abs_addr = waddr.addr;
			if (old_abs_addr > new_abs_addr)
				waddr.page++;
			int pages = type == FLASH ? memc->flash_pages : memc->ram_pages;
			waddr.page %= pages;
			break;
		}
	}
	return waddr;
}

/* returns number of bytes read */
int disassemble(CALC* lpCalc, ViewType type, waddr_t waddr, int count, Z80_info_t *result) {
	int i, prefix = 0, pi = 0;
	memory_context_t *memc = &lpCalc->mem_c;
	for (i = 0; i < count; i++, result++, prefix = 0) {
		waddr_t start_addr = result->waddr = waddr;

		char *labelname = FindAddressLabel(lpCalc, waddr);

		if (labelname) {
			result->index = DA_LABEL;
			result->a1 = (uint32_t *) labelname;
			result->size = 0;
			result++;
			result->waddr = waddr;
			if (++i >= count)
				break;
		}

		unsigned char data = wmem_read(memc, waddr);
		waddr = GetNextAddr(memc, type, waddr);
		
		if (data == 0xDD || data == 0xFD) {
			prefix = data;
			pi = (prefix >> 5) & 1;
			data = wmem_read(memc, waddr);
			waddr = GetNextAddr(memc, type, waddr);
		}
		if (data == 0xCB) {
			int offset;
			data = wmem_read(memc, waddr);
			waddr = GetNextAddr(memc, type, waddr);
			if (prefix) {
				offset = (char) data;
				data = wmem_read(memc, waddr);
				waddr = GetNextAddr(memc, type, waddr);
			}
			int x = (data & 0xC0) >> 6;
			int y = (data & 0x38) >> 3;
			int z = (data & 0x07);
			result->a1 = y;
			result->a2 = (uint32_t *) r[z];
			
			switch (x) {
				case 0: /* X = 0 */
					result->index = DA_ROT;
					result->a1 = (uint32_t *) rot[y];
					break;
				case 1:	result->index = (result->a2 == (uint32_t *) r[R__HL_] && !prefix) ? DA_BIT__HL_ : DA_BIT; break; /* X = 1 */
				case 2:	result->index = (result->a2 == (uint32_t *) r[R__HL_] && !prefix) ? DA_RES__HL_ : DA_RES; break; /* X = 2 */
				case 3:	result->index = (result->a2 == (uint32_t *) r[R__HL_] && !prefix) ? DA_SET__HL_ : DA_SET; break; /* X = 3 */
			}
			if (prefix) {
				char *flagname = nullptr, *bitname = nullptr;
				FindFlags(offset, y, &flagname, &bitname);

				/* Special IY flags*/
				if ((prefix == 0xFD) &&
					(x != 0) &&
					(lpCalc->cpu.iy == 0x89F0) &&
					(lpCalc->model >= TI_83P) &&
					lpCalc->bTIOSDebug &&
					flagname && bitname) {
					if (z == 6) {
						result->index += (DA_BIT_IF - DA_BIT);
						result->a1 = (uint32_t *) bitname;
						result->a2 = (uint32_t *) ("iy");
						result->a3 = (uint32_t *) flagname;
					} else {
						result->index += (DA_BIT_RF - DA_BIT);
						result->a4 = result->a2;	// old register target receives the result
						result->a1 = (uint32_t *) bitname;
						result->a2 = (uint32_t *) ("iy");
						result->a3 = (uint32_t *) flagname;
					}
				} else {
					if (z == 6) {
						result->index += (DA_ROT_I - DA_ROT);
						result->a2 = (uint32_t *) ri[pi];
						result->a3 = offset;
					} else {
						result->index += (DA_ROT_R - DA_ROT);
						result->a4 = result->a2;
						result->a3 = offset;
						result->a2 = (uint32_t *) ri[pi];
					}
				}
			}
		} else if (data == 0xED) {
			data = wmem_read(memc, waddr);
			waddr = GetNextAddr(memc, type, waddr);
			int x = (data & 0xC0) >> 6;
			int y = (data & 0x38) >> 3;
			int z = (data & 0x07);
			int p = (data & 0x30) >> 4;
			int q = y & 1;
			
			if (x == 1) {
			/* FOR X  = 1 */
				if (z == 0) {
					if (y == 6) y = 8;
					result->index = DA_IN_R__C_;
					result->a1 = (uint32_t *) r[y];
					result->a2 = (uint32_t *) r[R_C];
				} else if (z == 1) {
					if (y == 6) y = 8;
					result->index = DA_OUT__C__R;
					result->a1 = (uint32_t *) r[R_C];
					result->a2 = (uint32_t *) r[y];
				} else if (z == 2) {
					if (q == 0) result->index = DA_SBC_HL_RP;
					else result->index = DA_ADC_HL_RP;
					result->a1 = (uint32_t *) rp[R_HL];
					result->a2 = (uint32_t *) rp[p];
				} else if (z == 3) {
					if (q == 0) {
						result->index = DA_LD__X__RP;
						result->a1 = wmem_read16(memc, waddr);
						waddr = GetNextAddr(memc, type, waddr);
						waddr = GetNextAddr(memc, type, waddr);
						result->a2 = (uint32_t *) rp[p];
					} else {
						result->index = DA_LD_RP__X_;
						result->a1 = (uint32_t *) rp[p];
						result->a2 = wmem_read16(memc, waddr);
						waddr = GetNextAddr(memc, type, waddr);
						waddr = GetNextAddr(memc, type, waddr);
					}
				} else if (z == 4) {
					result->index = DA_NEG;
				} else if (z == 5) {
					if (y == 1) {
						result->index = DA_RETI;
					} else {
						result->index = DA_RETN;
					}
				} else if (z == 6) {
					result->index = DA_IM_X;
					result->a1 = (uint32_t *) im[y];
				} else if (z == 7) {
					switch (y) {
						case 0:	result->index = DA_LD_I_A; break;
						case 1: result->index = DA_LD_R_A; break;
						case 2: result->index = DA_LD_A_I; break;
						case 3: result->index = DA_LD_A_R; break;
						case 4: result->index = DA_RRD; break;
						case 5: result->index = DA_RLD; break;
						default:
							result->index = DA_NOP_ED; break;
					}
				}
			} else
			/* FOR X  = 2 */
			if (x == 2) {
				if (y >= 4) {
					result->index = DA_BLI;
					result->a1 = (uint32_t *) bli[y-4][z];
				} else {
					result->index = DA_NOP_ED;
				}
			} else {
				result->index = DA_NOP_ED;
			}
		} else {
			int x = (data & 0xC0) >> 6;
			int y = (data & 0x38) >> 3;
			int z = (data & 0x07);
			int p = (data & 0x30) >> 4;
			int q = y & 1;
			int offset = wmem_read(memc, waddr);
				
			if (x == 0) {
				
				
			/* FOR X  = 0 */
			if (z == 0) {
				switch (y) {
					case 0: result->index = DA_NOP; break;
					case 1: 
						result->index = DA_EX_AF_AF_;
						result->a1 = (uint32_t *) rp2[3];
						result->a2 = (uint32_t *) ("af'");
						break;
					case 2: 
						result->index = DA_DJNZ_X;
						result->a1 = wmem_read(memc, waddr);
						waddr = GetNextAddr(memc, type, waddr);
						break;
					case 3: 
						result->index = DA_JR_X; 
						result->a1 = wmem_read(memc, waddr);
						waddr = GetNextAddr(memc, type, waddr);
						break;
					default:
						result->index = DA_JR_CC_X;
						result->a1 = (uint32_t *) cc[y-4];
						result->a2 = wmem_read(memc, waddr);
						waddr = GetNextAddr(memc, type, waddr);
						break;
				}
			} else
			if (z == 1) {	/* ix, iy ready */
				if (q == 0) {
					result->index = DA_LD_RP_X;
					result->a2 = wmem_read16(memc, waddr);
					waddr = GetNextAddr(memc, type, waddr);
					waddr = GetNextAddr(memc, type, waddr);
					if (prefix && p == 2) {
						result->a1 = (uint32_t *) ri[pi];
					} else {
						result->a1 = (uint32_t *) rp[p];
					}
				} else {
					if (prefix) {
						result->index = DA_ADD_RI_RP;
						result->a1 = (uint32_t *) ri[pi];
						result->a2 = (uint32_t *) rpi[pi][p];
					} else {
						result->index = DA_ADD_HL_RP;
						result->a1 = (uint32_t *) rp[R_HL];
						result->a2 = (uint32_t *) rp[p];
					}
				}
			} else
			if (z == 2) {	/* ix, iy ready */
				switch (y) {
					case 0: 
						result->index = DA_LD__BC__A; 
						result->a1 = (uint32_t *) rp[R_BC];
						result->a2 = (uint32_t *) r[R_A];
						break;
					case 1: 
						result->index = DA_LD_A__BC_;
						result->a1 = (uint32_t *) r[R_A];
						result->a2 = (uint32_t *) rp[R_BC];
						break;
					case 2: 
						result->index = DA_LD__DE__A; 
						result->a1 = (uint32_t *) rp[R_DE];
						result->a2 = (uint32_t *) r[R_A];
						break;
					case 3: 
						result->index = DA_LD_A__DE_; 
						result->a1 = (uint32_t *) r[R_A];
						result->a2 = (uint32_t *) rp[R_DE];
						break;
					case 4: 
						if (prefix) {
							result->index = DA_LD__X__RI;
							result->a2 = (uint32_t *) ri[pi];
							result->a1 = wmem_read16(memc, waddr);
							waddr = GetNextAddr(memc, type, waddr);
							waddr = GetNextAddr(memc, type, waddr);
						} else {
							result->index = DA_LD__X__HL;
							result->a1 = wmem_read16(memc, waddr);
							waddr = GetNextAddr(memc, type, waddr);
							waddr = GetNextAddr(memc, type, waddr);
							result->a2 = (uint32_t *) rp[R_HL];
						}
						break;							
					case 5:
						if (prefix) {
							result->index = DA_LD_RI__X_;
							result->a1 = (uint32_t *) ri[pi];
							result->a2 = wmem_read16(memc, waddr);
							waddr = GetNextAddr(memc, type, waddr);
							waddr = GetNextAddr(memc, type, waddr);
						} else {
							result->index = DA_LD_HL__X_;
							result->a1 = (uint32_t *) rp[R_HL];
							result->a2 = wmem_read16(memc, waddr);
							waddr = GetNextAddr(memc, type, waddr);
							waddr = GetNextAddr(memc, type, waddr);
							
						}
						break;
					case 6:	result->index = DA_LD__X__A;
							result->a1 = wmem_read16(memc, waddr);
							waddr = GetNextAddr(memc, type, waddr);
							waddr = GetNextAddr(memc, type, waddr);
							result->a2 = (uint32_t *) r[R_A];
							break;
					case 7:	result->index = DA_LD_A__X_;
							result->a1 = (uint32_t *) r[R_A];
							result->a2 = wmem_read16(memc, waddr);
							waddr = GetNextAddr(memc, type, waddr);
							waddr = GetNextAddr(memc, type, waddr);
							break;	
				}				
			} else
			if (z == 3) {	/* ix, iy ready */
				result->index = (q == 0) ? DA_INC_RP : DA_DEC_RP;
				result->a1 = (uint32_t *) (prefix ? rpi[pi][p] : rp[p]);
			} else
			if (z < 6) {	/* ix, iy ready */
				result->index = (z == 4) ? DA_INC_R : DA_DEC_R;
				result->a1 = (uint32_t *) (prefix ? r8i[pi][y] : r[y]);
				if (result->a1 == (uint32_t *) r[R__HL_])
					result->index = (z == 4) ? DA_INC__HL_ : DA_DEC__HL_;
				if (prefix && y == 6) {
					result->index += (DA_INC_RI - DA_INC_R);
					result->a2 = offset;
					waddr = GetNextAddr(memc, type, waddr);
				}
					
			} else
			if (z == 6) {	/* ix, iy ready */
				result->index = DA_LD_R_X;
				result->a1 = (uint32_t *) (prefix ? r8i[pi][y] : r[y]);
				if (result->a1 == (uint32_t *) r[R__HL_])
					result->index = DA_LD__HL__X;
				if (prefix && y == 6) {
					result->index = DA_LD_RI_X;
					result->a2 = offset;
					waddr = GetNextAddr(memc, type, waddr);
					result->a3 = (uint32_t *) wmem_read(memc, waddr);
					waddr = GetNextAddr(memc, type, waddr);
				} else {
					result->a2 = (uint32_t *) wmem_read(memc, waddr);
					waddr = GetNextAddr(memc, type, waddr);
				}
			} else {	/* ix, iy ready */
				switch (y) {
					case 0:	result->index = DA_RLCA; break;
					case 1: result->index = DA_RRCA; break;
					case 2: result->index = DA_RLA; break;
					case 3: result->index = DA_RRA; break;
					case 4: result->index = DA_DAA; break;
					case 5: result->index = DA_CPL; break;
					case 6: result->index = DA_SCF; break;
					case 7: result->index = DA_CCF; break;
				}
			}
			} else
			
			
			/* FOR X = 1 */
			if (x == 1) {	/* ix, iy ready */
				if (z == 6 && y == 6) {
					result->index = DA_HALT;
				} else {
					result->index = DA_LD_R_R;
					result->a1 = (uint32_t *) (prefix ? r8i[pi][y] : r[y]);
					if (result->a1 == (uint32_t *) r[R__HL_])
						result->index = DA_LD__HL__R;
					result->a2 = (uint32_t *) (prefix ? r8i[pi][z] : r[z]);
					if (result->a2 == (uint32_t *) r[R__HL_])
						result->index = DA_LD_R__HL_;
					if (prefix) {
						if (y == 6) {
							waddr = GetNextAddr(memc, type, waddr);
							result->index = DA_LD_RI_R;
							result->a1 = (uint32_t *) ri[pi];
							result->a3 = (uint32_t *) r[z];
							result->a2 = offset;
						} else if (z == 6) {
							waddr = GetNextAddr(memc, type, waddr);
							result->a1 = (uint32_t *) r[y];
							result->a2 = (uint32_t *) ri[pi];
							result->index = DA_LD_R_RI;
							result->a3 = offset;
						}
					}			
				}
			} else
			/* FOR X = 2 */
			if (x == 2) {	/* ix, iy ready */
				uint32_t * *a = result->a;
				*(a++) = (uint32_t *) alu[y];
				if (y == 0 || y == 1 || y == 3) {
					*(a++) = (uint32_t *) r[R_A];
					result->index = DA_ALU_A;
				} else
				result->index = DA_ALU;
				
				*(a++) = (uint32_t *) (prefix ? r8i[pi][z] : r[z]);
				if (prefix && z == 6) {
					result->index += (DA_ALU_RI - DA_ALU);
					*(a++) = offset;
					waddr = GetNextAddr(memc, type, waddr);
				}
					
			} else
			if (x == 3) {
			/* FOR X = 3 */
			if (z == 0) {
				result->index = DA_RET_CC;
				result->a1 = (uint32_t *) cc[y];
			} else
			if (z == 1) {
				if (q == 0) {
					result->index = DA_POP_RP;
					result->a1 = (uint32_t *) (prefix ? rp2i[pi][p] : rp2[p]);
				} else {
					switch (p) {
						case 0:	result->index = DA_RET; break;
						case 1: result->index = DA_EXX; break;
						case 2:
							result->index = prefix ? DA_JP_RI : DA_JP_HL;
							result->a1 = (uint32_t *) (prefix ? ri[pi] : rp[R_HL]);
							break;
						case 3: 
							result->index = DA_LD_SP_HL;
							result->a1 = (uint32_t *) rp[R_SP];
							result->a2 = (uint32_t *) (prefix ? ri[pi] : rp[R_HL]);
							break;
					}
				}
			} else
			if (z == 2) {
				result->index = DA_JP_CC_X;
				result->a1 = (uint32_t *) cc[y];
				result->a2 = wmem_read16(memc, waddr);
				waddr = GetNextAddr(memc, type, waddr);
				waddr = GetNextAddr(memc, type, waddr);
			} else
			if (z == 3) {
				switch (y) {
					case 0:
						result->index = DA_JP_X;
						result->a1 = wmem_read16(memc, waddr);
						waddr = GetNextAddr(memc, type, waddr);
						waddr = GetNextAddr(memc, type, waddr);
						break;
					case 2:
						result->index = DA_OUT__X__A;
						result->a1 = wmem_read(memc, waddr);
						waddr = GetNextAddr(memc, type, waddr);
						result->a2 = (uint32_t *) r[R_A];
						break;
					case 3:
						result->index = DA_IN_A__X_;
						result->a1 = (uint32_t *) r[R_A];
						result->a2 = wmem_read(memc, waddr);
						waddr = GetNextAddr(memc, type, waddr);
						break;
					case 4:	
						result->index = (prefix ? DA_EX__SP__RI : DA_EX__SP__HL);
						result->a1 = (uint32_t *) rp[R_SP];
						result->a2 = (uint32_t *) (prefix ? ri[pi] : rp[R_HL]);
						break;
					case 5: 
						result->index = DA_EX_DE_HL; 
						result->a1 = (uint32_t *) rp[R_DE];
						result->a2 = (uint32_t *) rp[R_HL];
						break;
					case 6: result->index = DA_DI; break;
					case 7: result->index = DA_EI; break;
				}
			} else
			if (z == 4) {
				result->index = DA_CALL_CC_X;
				result->a1 = (uint32_t *) cc[y];
				result->a2 = wmem_read16(memc, waddr);
				waddr = GetNextAddr(memc, type, waddr);
				waddr = GetNextAddr(memc, type, waddr);
			} else
			if (z == 5) {
				if (q == 0) {
					result->index = DA_PUSH_RP;
					result->a1 = (uint32_t *) (prefix ? rp2i[pi][p] : rp2[p]);
				} else {
					if (p == 0) {
						result->index = DA_CALL_X;
						result->a1 = wmem_read16(memc, waddr);
						waddr = GetNextAddr(memc, type, waddr);
						waddr = GetNextAddr(memc, type, waddr);

						if ((result->a1 == 0x0050) && lpCalc->model >= TI_83P && lpCalc->bTIOSDebug) {
							result->index = DA_BJUMP;
							result->a1 = wmem_read16(memc, waddr);
							waddr = GetNextAddr(memc, type, waddr);
							waddr = GetNextAddr(memc, type, waddr);
							char* Name = FindBcall((int) result->a1);
							if (Name == nullptr) {
								result->index = DA_BJUMP_N;
							} else {
								result->a1 = (uint32_t *) Name;
							}
						}

					}
				}					
			} else
			if (z == 6) {
				switch (y) {
					case 0:
						result->index = DA_ADD_HL_RP;
						result->a1 = (uint32_t *) r[R_A];
						break;
					case 1:
						result->index = DA_ADC_HL_RP;
						result->a1 = (uint32_t *) r[R_A];
						break;
					case 3:
						result->index = DA_SBC_HL_RP;
						result->a1 = (uint32_t *) r[R_A];
						break;
					default:
						result->index = DA_ALU;
						result->a1 = (uint32_t *) alu[y];
						break;
				}
				result->index = DA_ALU_X;
				result->a1 = (uint32_t *) alu[y];
				result->a2 = (uint32_t *) wmem_read(memc, waddr);
				waddr = GetNextAddr(memc, type, waddr);
			} else
			if (z == 7) {
				if ((y == 5) && (lpCalc->model >= TI_83P) && lpCalc->bTIOSDebug) {
					result->index = DA_BCALL;
					int tmp = wmem_read16(memc, waddr);
						waddr = GetNextAddr(memc, type, waddr);
						waddr = GetNextAddr(memc, type, waddr);
					char* Name = FindBcall(tmp);
					if (Name == nullptr) {
						result->index = DA_BCALL_N;
						result->a1 = tmp;
					} else {
						result->a1 = (uint32_t *) Name;
					}
				} else {
					result->index = DA_RST_X;
					result->a1 = y*8;
				}
			}
			}
		}
		result->size = abs(((unsigned short)(waddr.addr - start_addr.addr)) & 0xFF);

#ifndef WINVER
		uint32_t * mod_a1 = result->a1;
		uint32_t * mod_a2 = result->a2;

		// Expand the format
		char szFormat[32] = {0};
		char *in = da_opcode[result->index].format;
		char *out = szFormat;
		int inOffset = 0;
		int outOffset = 0;
		while (in[inOffset] != ('\0'))
		{
			if (in[inOffset] == ('%'))
			{
				switch (in[inOffset + 1])
				{
				case ('g'):
					{
						unsigned short addr = result->waddr.addr + 2;
						if (result->index == DA_JR_CC_X)
							mod_a2 = (addr + ((char) result->a2)) & 0xFFFF;
						else
							mod_a1 = (addr + ((char) result->a1)) & 0xFFFF;
						// Fall through
					}
				case ('a'):
					{
						const char * sz = ("$%04X");
						_tcscat(out, sz);
						outOffset += strlen(sz);
						inOffset += 2;
						break;
					}
				case ('h'):
					{
						const char * sz = ("%+d");
						_tcscat(out, sz);
						outOffset += strlen(sz);
						inOffset += 2;
						break;
					}
				case ('r'):
				case ('c'):
				case ('l'):
					{
						out[outOffset++]  = in[inOffset++];
#ifdef _UNICODE
						out[outOffset] = ('S');
#else
						out[outOffset] = ('s');
#endif
						outOffset++;
						inOffset++;
						break;
					}
				case ('x'):
					{
						const char * sz = ("$%02X");
						_tcscat(out, sz);
						outOffset += strlen(sz) ;
						inOffset += 2;
						break;
					}
				case ('s'):
					{
						out[outOffset++] = in[inOffset++];
#ifdef _UNICODE
						out[outOffset++] = ('S');
						inOffset++;
#else
						out[outOffset++] = in[inOffset++];
#endif
						break;
					}
				default:
					{
						
						out[outOffset++] = in[inOffset++];
						out[outOffset++] = in[inOffset++];
						break;
					}
				}
			}
			else
			{
				out[outOffset++] = in[inOffset++];
			}
		}
		_tprintf(result->expanded, szFormat, mod_a1, mod_a2, result->a3,result->a4);
#endif
	}
	
	return i;
}
