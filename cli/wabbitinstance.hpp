//
//  wabbitinstance.hpp
//  CLI WabbitEmu
//
//  Created by Thomas Dickerson on 9/24/15.
//  Copyright © 2015 StickFigure Graphic Productions. All rights reserved.
//

#ifndef wabbitinstance_hpp
#define wabbitinstance_hpp

#include <stdio.h>


#define MAX_FILES 255
typedef struct ParsedCmdArgs
{
	const char * rom_files[MAX_FILES];
	const char * utility_files[MAX_FILES];
	const char * archive_files[MAX_FILES];
	const char * ram_files[MAX_FILES];
	int num_rom_files;
	int num_utility_files;
	int num_archive_files;
	int num_ram_files;
	bool silent_mode;
	bool force_new_instance;
	bool force_focus;
} ParsedCmdArgs_t;

class WabbitInstance {
	
};

#endif /* wabbitinstance_hpp */
