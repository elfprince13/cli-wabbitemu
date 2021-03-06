#include "stdafx.h"
#include "gui.h"
#include "calc.h"
#include <wx/config.h>


#define MAX_FILES 255
typedef struct ParsedCmdArgs
{
	LPTSTR rom_files[MAX_FILES];
	LPTSTR utility_files[MAX_FILES];
	LPTSTR archive_files[MAX_FILES];
	LPTSTR ram_files[MAX_FILES];
	int num_rom_files;
	int num_utility_files;
	int num_archive_files;
	int num_ram_files;
	bool silent_mode;
	bool force_new_instance;
	bool force_focus;
} ParsedCmdArgs_t;


class WabbitemuApp: public wxApp
{
private:
	virtual bool OnInit();
	virtual int OnExit();
	void OnTimer(wxTimerEvent& event);
	void getTimer(int slot);
	void LoadSettings(CALC* lpCalc);
	void SaveSettings(CALC* lpCalc);
	wxConfigBase *settingsConfig;
	wxTimer *timer;
	unsigned GetTickCount();
	
	ParsedCmdArgs_t parsedArgs;
	void ParseCommandLineArgs();
	void LoadCommandlineFiles(uint32_t *,  void (*load_callback)(uint32_t *, LPTSTR, SEND_FLAG));
public:
	static bool DoRomWizard();
};

void LoadToCALC*(uint32_t * lParam, LPTSTR filePath, SEND_FLAG sendLoc);