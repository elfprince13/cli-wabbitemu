#include "../core/coretypes.hpp"

#include "fileutilities.hpp"
#include "../libs/libcrane/include/crane.h"


// These are terrifyingly unsafe.
int BrowseFile(char* file, const char *filter, const char *title, const char *defExt, unsigned int flags) {
	file[0] = '\0';
	return 0;
}

int SaveFile(char *file, const char *filter, const char *title, const char defExt, unsigned int flags, unsigned int filterIndex) {
	file[0] = '\0';
	return 0;
}

// This is a job for libcrane
void GetAppDataString(char *buffer, size_t len) {
	getApplicationDataDirectory(buffer, len, "wabbit-cli");

}