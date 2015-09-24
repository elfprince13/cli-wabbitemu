#include "../interface/calc.hpp"
#include "../hardware/link.hpp"
#include "label.hpp"

//Sends a file to the given calculator
//from the given filename
LINK_ERR SendFile(/*const*/ CALC* lpCalc, const char * lpszFileName, SEND_FLAG Destination)
{
	TIFILE_t *var = newimportvar(lpszFileName, false);

	LINK_ERR result;
	if (var != nullptr)
	{
		switch(var->type)
		{
		case GROUP_TYPE:
		case VAR_TYPE:
		case FLASH_TYPE:
			{
				if (var->type == FLASH_TYPE)
					lpCalc->running = false;
				lpCalc->cpu.pio.link->vlink_size = var->length;
				lpCalc->cpu.pio.link->vlink_send = 0;

				result = link_send_var(&lpCalc->cpu, var, (SEND_FLAG) Destination);
				if (var->type == FLASH_TYPE)
				{
					// Rebuild the applist
					state_build_applist(&lpCalc->cpu, &lpCalc->applist);

					uint32_t i;
					for (i = 0; i < lpCalc->applist.count; i++) {
						if (strncmp((char *) var->flash->name, lpCalc->applist.apps[i].name, 8) == 0) {
							lpCalc->last_transferred_app = &lpCalc->applist.apps[i];
							break;
						}
					}
					if (var->flash->type == FLASH_TYPE_OS) {
						calc_reset(lpCalc);
						//calc_turn_on(lpCalc);
					}
					lpCalc->running = true;
				}
				break;
			}
		case BACKUP_TYPE:
			lpCalc->cpu.pio.link->vlink_size = var->length;
			lpCalc->cpu.pio.link->vlink_send = 0;
			result = link_send_backup(&lpCalc->cpu, var, (SEND_FLAG) Destination);
			break;
		case ROM_TYPE:
		case SAV_TYPE:
			{
				FreeTiFile(var);
				var = nullptr;
				if (rom_load(lpCalc, lpszFileName) == true) {
					result = LERR_SUCCESS;
				} else {
					result = LERR_LINK;
				}
				break;
			}
		case LABEL_TYPE:
			{
				strcpy(lpCalc->labelfn, lpszFileName);
				VoidLabels(lpCalc);
				labels_app_load(lpCalc, lpCalc->labelfn);
				result = LERR_SUCCESS;
				break;
			}
		case BREAKPOINT_TYPE:
			break;
		}
		if (var)
		{
			FreeTiFile(var);
		}

		return result;
	}
	else
	{
		return LERR_FILE;
	}
}
