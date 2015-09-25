//
//  wabbitinstance.cpp
//  CLI WabbitEmu
//
//  Created by Thomas Dickerson on 9/24/15.
//  Copyright © 2015 StickFigure Graphic Productions. All rights reserved.
//

#include "wabbitinstance.hpp"
#include "../hardware/link.hpp"
#include "../core/coretypes.hpp"


bool WabbitInstance::parseArgs(const char **argv, size_t argc) {
	memset(&parsedArgs, 0, sizeof(ParsedCmdArgs));
	char tmpstring[512];
	SEND_FLAG ram = SEND_CUR;
	
	bool ret;
	if (argv && argc > 1) {
		strcpy(tmpstring, argv[1]);
		for(int i = 1; i < argc; i++) {
			memset(tmpstring, 0, 512);
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
		ret = true;
	}
	
	return true;
}