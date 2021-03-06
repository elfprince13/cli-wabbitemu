#ifndef ROMWIZARD_H
#define ROMWIZARD_H
#include "wizard/wizardstart.h"
#include "wizard/wizardcalctype.h"
#include "wizard/wizardos.h"
#include <wx/wx.h>
#include <wx/url.h>
#include <wx/wfstream.h>

class RomWizard : public wxWizard {
private:
	WizardStartPage *startPage;
	WizardCalcTypePage *calcTypePage;
	WizardOSPage *osPage;
	void OnFinish(wxWizardEvent &);
	void OnPageChanged(wxWizardEvent &);
	void ModelInit(CALC* lpCalc, int model);
	bool ExtractBootFree(wxString &bootfreePath, int model);
	bool DownloadOS(wxString &osFilePath, int model, bool version);
protected:
	DECLARE_EVENT_TABLE()
public:
	RomWizard();
	bool Begin();
};

#endif