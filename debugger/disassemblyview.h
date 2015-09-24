#ifndef DISASSEMBLYVIEW_H
#define DISASSEMBLYVIEW_H
#include <wx/wx.h>
#include <wx/listctrl.h>
#include "debuggerwindowclass.h"
#include "disassemble.h"

class DisassemblyView: public wxListCtrl, DebuggerWindowClass {
public:
	DisassemblyView(wxWindow *parent, CALC* lpCalc, ViewType type);
	wxString OnGetItemText(long item, long column) const;
	wxListItemAttr * OnGetItemAttr(long item) const;
	void GotoAddress(waddr_t waddr);
	ViewType viewType;
	waddr_t& MapIndexToAddress(int index);
	int MapAddressToIndex(waddr_t &address);
	
	void DebugUpdateWindow();
private:
	CALC* lpCalc;
	void sprint_data(CALC* lpCalc, const Z80_info_t *zinf, char *s) const;
	void sprint_addr(CALC* lpCalc, const Z80_info_t *zinf, char *s) const;
	void sprint_command(CALC* lpCalc, const Z80_info_t *zinf, char *s) const;
	void sprint_size(CALC* lpCalc, const Z80_info_t *zinf, char *s) const;
	void sprint_clocks(CALC* lpCalc, const Z80_info_t *zinf, char *s) const;
	int FindLastItem();
	Z80_info_t *zinf;
};
#endif