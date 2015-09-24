#include "print.hpp"
#include "label.hpp"
#include "../core/core.hpp"


#define press_text(sztext, zcolor) press_textA(sztext, zcolor, &r, hdc)

static bool calc_size = false;
static size_t mspf_size = 0;
static int mspf_break = 9999;

void press_textA(char *szText, uint32_t zcolor, RECT *r, HDC hdc) {
	RECT tr;

	tr.left = 0; tr.right = 1;
	SetTextColor(hdc, zcolor);
	DrawText(hdc, szText, -1, &tr, DT_LEFT | DT_SINGLELINE | DT_CALCRECT);
	r->right = r->left + tr.right;
	
	size_t index = mspf_size;
	mspf_size += (int) strlen(szText);
	if (calc_size == false) {
		const char *dot_strings[] = {("."), (".."), ("...")};
		char szNew[1024];
		
		if (index >= mspf_break || (index < mspf_break && index+strlen(szText) > mspf_break)) {
			int break_index = (int) (max(index, mspf_break));
			int break_string_index = break_index - (int) index;
			int str_left = (int) _tclen(&szText[break_string_index]);
			
			if (str_left > 3)
				str_left = 3;

			if (index > mspf_break)
				str_left -= (int) (index - mspf_break);
			
			if (str_left < 1)
				str_left = 1;
			
			strcpy(szNew, szText);
			strcpy(&szNew[break_string_index], dot_strings[str_left-1]);
			
			szText = szNew;
		}
		
		DrawText(hdc, szText, -1, r, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
	}
	OffsetRect(r, tr.right, 0);
}


void MyDrawText(CALC* lpCalc, HDC hdc, RECT *rc, Z80_info_t* zinf, ViewType type, const char *fmt, ...) {
	char *p;
	va_list argp;
	RECT r = *rc;
	
	mspf_size = 0;
	mspf_break = 999;
	
	if (calc_size == false) {
		calc_size = true;
		
		MyDrawText(lpCalc, hdc, rc, zinf, REGULAR, fmt, zinf->a1, zinf->a2, zinf->a3, zinf->a4);
		
		char szFilltext[1024];
		memset(szFilltext, 'A', mspf_size);
		szFilltext[mspf_size] = '\0';

		RECT hr;
		CopyRect(&hr, rc);
		DrawText(hdc, szFilltext, -1, &hr, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_CALCRECT | DT_END_ELLIPSIS | DT_MODIFYSTRING);

		mspf_break = (int) strlen(szFilltext);

		if (mspf_break < mspf_size) {
			mspf_break -= 3;
		} else {
			mspf_break++;
		}
		calc_size = false;
	}
	
	mspf_size = 0;
	
	// Initialize arguments
	va_start(argp, fmt);
	for (p = (char *) fmt; *p && (mspf_size < mspf_break+3); p++) {
		if(*p != '%') {
			char szChar[2] = ("x");
			szChar[0] = *p;
			press_text(szChar, DBCOLOR_BASE);
		} else {
			switch(*++p) {
				case 'c': {		//condition
					char *s = va_arg(argp, char *);
					press_text(s, DBCOLOR_CONDITION);
					break;
				}
				case 'h': {		//offset
					int val	= (int) va_arg(argp, uint32_t *);
					char szOffset[8];
					if (val & 0x80) {
						_stprintf_s(szOffset, ("%d"), val - 256);
					} else {
						_stprintf_s(szOffset, ("+%d"), val);
					}

					press_text(szOffset, RGB(0, 0, 0));
					break;
				}
				case 'd': {		//number
					int val	= (int) va_arg(argp, uint32_t *);
					char szAddr[16];
					_stprintf_s(szAddr, ("%d"), val);

					press_text(szAddr, RGB(0, 0, 0));		
					break;
				}
				case 'l':
				{
					char *s = va_arg(argp, char *);
					press_text(s, RGB(0, 0, 0));
					break;
				}		
				case 's':
				{
					char *s = va_arg(argp, char *);
					press_text(s, DBCOLOR_BASE);
					break;
				}
				case 'g':
				{
					waddr_t waddr = OffsetWaddr(lpCalc->cpu.mem_c, REGULAR, zinf->waddr, 2 + ((char) va_arg(argp, uint32_t *)));
					char *name;
					
					name = FindAddressLabel(lpCalc, waddr);
					
					if (name) {
						press_text(name, RGB(0, 0, 0));
					} else {
						char szAddr[16];
						_stprintf_s(szAddr, ("$%04X"), waddr.addr);
						press_text(szAddr, RGB(0, 0, 0));
					}
					break;
				}
				case 'a': //address
					{
						waddr_t waddr = OffsetWaddr(lpCalc->cpu.mem_c, REGULAR, zinf->waddr, 2);
						char *name;
						int val = (int) va_arg(argp, uint32_t *);

						name = FindAddressLabel(lpCalc, addr_to_waddr(lpCalc->cpu.mem_c, val));
						
						if (name) {
							press_text(name, RGB(0, 0, 0));
						} else {
							char szAddr[16];
							_stprintf_s(szAddr, ("$%04X"), val);
							press_text(szAddr, RGB(0, 0, 0));
						}
						break;
					}
				case 'r':
				{
					char *szReg = va_arg(argp, char *);
					if (!_tcscmp(szReg, ("(hl)"))) {
						press_text(("("), DBCOLOR_BASE);
						press_text(("hl"), DBCOLOR_HILIGHT);
						press_text((")"), DBCOLOR_BASE);
					} else
						press_text(szReg, DBCOLOR_HILIGHT);
					break;
				}
				case 'x':
				{
					int val	= (int) va_arg(argp, uint32_t *);
					char szAddr[16];
					StringCbPrintf(szAddr, sizeof(szAddr), ("$%02X"), val);
					press_text(szAddr, RGB(0, 0, 0));	
					break;	
				}
			}
		}
	}
	va_end(argp);
}

char* mysprintf(CALC* lpCalc, Z80_info_t* zinf, ViewType type, const char *fmt, ...) {
	char *p;
	static char end_buf[1024] = ("\0");
	va_list argp;
	
	mspf_size = 0;
	mspf_break = 999;
	
	mspf_size = 0;
	
	// Initialize arguments
	va_start(argp, fmt);
	for (p = (char *) fmt; *p && (mspf_size < mspf_break+3); p++) {
		if(*p != '%') {
			char szChar[2] = ("x");
			szChar[0] = *p;
			StringCbCat(end_buf, sizeof(end_buf), szChar);
		} else {
			switch(*++p) {
				case 'c': {//condition
					char *s = va_arg(argp, char *);
					StringCbCat(end_buf, sizeof(end_buf), s);
					break;
				}
				case 'h': {//offset
					int val	= (int) va_arg(argp, uint32_t *);
					char szOffset[8];
#ifdef WINVER
					_stprintf_s(szOffset, ("%+d"),val);
#else
					sprintf(szOffset, "%+d",val);
#endif
					StringCbCat(end_buf, sizeof(end_buf), szOffset);
					break;
				}
				case 'd': //number
				{
					int val	= (int) va_arg(argp, uint32_t *);
					char szAddr[16];
#ifdef WINVER
					_stprintf_s(szAddr, ("%d"), val);
#else
					sprintf(szAddr, "%d",val);
#endif
					StringCbCat(end_buf, sizeof(end_buf), szAddr);		
					break;
				}
				case 'l':
				{
					char *s = va_arg(argp, char *);
					StringCbCat(end_buf, sizeof(end_buf), s);
					break;
				}		
				case 's':
				{
					char *s = va_arg(argp, char *);
					StringCbCat(end_buf, sizeof(end_buf), s);
					break;
				}
				case 'g':
				{
					waddr_t waddr = OffsetWaddr(lpCalc->cpu.mem_c, type, zinf->waddr, 2 + (char) va_arg(argp, uint32_t *));
					char *name;

					name = FindAddressLabel(lpCalc, waddr);
					
					if (name) {
						StringCbCat(end_buf, sizeof(end_buf), name);
					} else {
						char szAddr[16];
						StringCbPrintf(szAddr, sizeof(szAddr), ("$%04X"), waddr.addr);
						StringCbCat(end_buf, sizeof(end_buf), szAddr);
					}
					break;
				}
				case 'a': //address
					{
						unsigned short addr = zinf->waddr.addr + 2;
						char *name;
						int val;
						val = (int) va_arg(argp, uint32_t *);

						name = FindAddressLabel(lpCalc, addr_to_waddr(lpCalc->cpu.mem_c, val));
						
						if (name) {
							StringCbCat(end_buf, sizeof(end_buf), name);
						} else {
							char szAddr[16];
							StringCbPrintf(szAddr, sizeof(szAddr), ("$%04X"), val);
							StringCbCat(end_buf, sizeof(end_buf), szAddr);
						}
						break;
					}
				case 'r':
				{
					char *szReg = va_arg(argp, char *);
					if (!_tcscmp(szReg, ("(hl)"))) {
						StringCbCat(end_buf, sizeof(end_buf), ("(hl)"));
					} else
					StringCbCat(end_buf, sizeof(end_buf), szReg);
					break;
				}
				case 'x':
				{
					int val	= (int) va_arg(argp, uint32_t *);
					char szAddr[16];
#ifdef WINVER
					StringCbPrintf(szAddr, sizeof(szAddr), ("$%02X"), val);
#else
					sprintf(szAddr, "$%02X", val);
#endif
					StringCbCat(end_buf, sizeof(end_buf), szAddr);
					break;	
				}
			}
		}
	}
	va_end(argp);
	return end_buf;
}