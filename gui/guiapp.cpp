#include "guiapp.h"
#include "gui.h"
#include "wizard/romwizard.h"
#include "sendfile.h"

WabbitemuFrame *frames[MAX_CALCS];

bool WabbitemuApp::DoRomWizard() {
	RomWizard wizard;	
	bool success = wizard.Begin();
	return success;
}

void WabbitemuApp::LoadSettings(CALC* lpCalc)
{
	settingsConfig = new wxConfig(wxT("Wabbitemu"));
	wxString tempString;
	settingsConfig->Read(wxT("/rom_path"), &tempString, wxEmptyString);
	strcpy(lpCalc->rom_path, tempString.c_str());
	settingsConfig->Read(wxT("/SkinEnabled"), &lpCalc->SkinEnabled, false);
}

bool WabbitemuApp::OnInit()
{
	wxImage::AddHandler(new wxPNGHandler);
	//stolen from the windows version
	ParseCommandLineArgs();

	memset(frames, 0, sizeof(frames));
	CALC* lpCalc = calc_slot_new();
	LoadSettings(lpCalc);
	
	WabbitemuFrame *frame;
	int result = rom_load(lpCalc, lpCalc->rom_path);
	if (result == true) {
		frame = gui_frame(lpCalc);
	} else {
		calc_slot_free(lpCalc);
		bool loadedRom = false;
		if (parsedArgs.num_rom_files > 0) {
			for (int i = 0; i < parsedArgs.num_rom_files; i++) {
				if (rom_load(lpCalc, parsedArgs.rom_files[i])) {
					gui_frame(lpCalc);
					loadedRom = true;
					break;
				}
			}
		}
		if (!loadedRom) {
			bool success = DoRomWizard();
			if (!success) {
				return false;
			}
		}
	}
	LoadCommandlineFiles((uint32_t *) lpCalc, LoadToCALC*);
	timer = new wxTimer();
	timer->Connect(wxEVT_TIMER, (wxObjectEventFunction) &WabbitemuApp::OnTimer);
	timer->Start(TPF, false);
	return true;
}

void WabbitemuApp::SaveSettings(CALC* lpCalc) {
#ifdef _UNICODE
	wxString rom_path(lpCalc->rom_path, wxConvUTF8);
#else
	wxString rom_path(lpCalc->rom_path);
#endif
	settingsConfig->Write(wxT("rom_path"), rom_path);
	settingsConfig->Write(wxT("SkinEnabled"), lpCalc->SkinEnabled);
	settingsConfig->Flush();
}

int WabbitemuApp::OnExit() {
	SaveSettings(&calcs[0]);
	//load ROMs first
	for (int i = 0; i < parsedArgs.num_rom_files; i++) {
		free(parsedArgs.rom_files[i]);
		parsedArgs.rom_files[i] = nullptr;
	}
	//then archived files
	for (int i = 0; i < parsedArgs.num_archive_files; i++) {
		free(parsedArgs.archive_files[i]);
		parsedArgs.archive_files[i] = nullptr;
	}
	//then ram
	for (int i = 0; i < parsedArgs.num_ram_files; i++) {
		free(parsedArgs.ram_files[i]);
		parsedArgs.ram_files[i] = nullptr;
	}
	//finally utility files (label, break, etc)
	for (int i = 0; i < parsedArgs.num_utility_files; i++) {
		free(parsedArgs.utility_files[i]);
		parsedArgs.utility_files[i] = nullptr;
	}
	return 0;
}


unsigned WabbitemuApp::GetTickCount()
{
		struct timeval tv;
		if(gettimeofday(&tv, nullptr) != 0)
			return 0;

		return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}


void WabbitemuApp::OnTimer(wxTimerEvent& event) {
	static int difference;
	static unsigned prevTimer;
	unsigned dwTimer = GetTickCount();
	
	// How different the timer is from where it should be
	// guard from erroneous timer calls with an upper bound
	// that's the limit of time it will take before the
	// calc gives up and claims it lost time
	difference += ((dwTimer - prevTimer) & 0x003F) - TPF;
	prevTimer = dwTimer;

	// Are we greater than Ticks Per Frame that would call for
	// a frame skip?
	if (difference > -TPF) {
		calc_run_all();
		while (difference >= TPF) {
			calc_run_all();
			difference -= TPF;
		}

		int i;
		for (i = 0; i < MAX_CALCS; i++) {
			if (calcs[i].active) {
				frames[i]->gui_draw();
			}
		}
	// Frame skip if we're too far ahead.
	} else {
		difference += TPF;
	}
}

void WabbitemuApp::ParseCommandLineArgs()
{
	ZeroMemory(&parsedArgs, sizeof(ParsedCmdArgs));
	char tmpstring[512];
	SEND_FLAG ram = SEND_CUR;

	if (argv && argc > 1) {
		strcpy(tmpstring, argv[1]);
		for(int i = 1; i < argc; i++) {
			ZeroMemory(tmpstring, 512);
			strcpy(tmpstring, argv[i]);
			char secondChar = toupper(tmpstring[1]);
			if (*tmpstring != '-' && *tmpstring != '/') {
				char *temp = (char *) malloc(strlen(tmpstring) + 1);
				strcpy(temp, tmpstring);
				temp[strlen(tmpstring) + 1] = '\0';
				char extension[5] = ("");
				const char *pext = strrchr(tmpstring, ('.'));
				if (pext != nullptr) {
					strcpy(extension, pext);
				}
				if (!strcasecmp(extension, (".rom")) || !strcasecmp(extension, (".sav")) || !strcasecmp(extension, (".clc"))) {
					parsedArgs.rom_files[parsedArgs.num_rom_files++] = temp;
				}
				else if (!strcasecmp(extension, (".brk")) || !strcasecmp(extension, (".lab")) 
					|| !strcasecmp(extension, (".zip")) || !strcasecmp(extension, (".tig"))) {
						parsedArgs.utility_files[parsedArgs.num_utility_files++] = temp;
				}
				else if (ram) {
					parsedArgs.ram_files[parsedArgs.num_ram_files++] = temp;
				} else {
					parsedArgs.archive_files[parsedArgs.num_archive_files++] = temp;
				}
			} else if (secondChar == 'R') {
				ram = SEND_RAM;
			} else if (secondChar == 'A') {
				ram = SEND_ARC;
			} else if (secondChar == 'S') {
				parsedArgs.silent_mode = true;
			} else if (secondChar == 'F') {
				parsedArgs.force_focus = true;
			} else if (secondChar == 'N') {
				parsedArgs.force_new_instance = true;
			}
		}
	}
}

/*void LoadAlreadyExistingWabbit(unsigned int lParam, LPTSTR filePath, SEND_FLAG sendLoc)
{
	HWND hwnd = (HWND) lParam;
	COPYDATASTRUCT *cds = (COPYDATASTRUCT *) malloc(sizeof(COPYDATASTRUCT));
	cds->dwData = sendLoc;
	size_t strLen;
	cds->lpData = filePath;
	if (PathIsRelative(filePath)) {
		char tempPath[PATH_MAX];
		char *tempPath2 = (char *) malloc(PATH_MAX);
		_tgetcwd(tempPath, PATH_MAX);
		PathCombine(tempPath2, tempPath, filePath);
		cds->lpData = tempPath2;
	}
	StringCbLength(filePath, 512, &strLen);
	cds->cbData = strLen;
	SendMessage(hwnd, WM_COPYDATA, (WPARAM) nullptr, (LPARAM) cds);
}*/

void LoadToCALC*(uint32_t * lParam, LPTSTR filePath, SEND_FLAG sendLoc)
{
	CALC* lpCalc = (CALC*) lParam;
	SendFile(lpCalc, filePath, sendLoc);
}

void WabbitemuApp::LoadCommandlineFiles(uint32_t * lParam,  void (*load_callback)(uint32_t *, LPTSTR, SEND_FLAG))
{
	//load ROMs first
	for (int i = 0; i < parsedArgs.num_rom_files; i++) {
		load_callback(lParam, parsedArgs.rom_files[i], SEND_ARC);
	}
	//then archived files
	for (int i = 0; i < parsedArgs.num_archive_files; i++) {
		load_callback(lParam, parsedArgs.archive_files[i], SEND_ARC);
	}
	//then ram
	for (int i = 0; i < parsedArgs.num_ram_files; i++) {
		load_callback(lParam, parsedArgs.ram_files[i], SEND_RAM);
	}
	//finally utility files (label, break, etc)
	for (int i = 0; i < parsedArgs.num_utility_files; i++) {
		load_callback(lParam, parsedArgs.utility_files[i], SEND_ARC);
	}
}