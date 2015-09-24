#ifndef FILEUTILITIES_H
#define FILEUTILITIES_H

#include <stdio.h>

int BrowseFile(char file, const char *filter, const char *title, const char *defExt, unsigned int flags = 0);
int SaveFile(char *lpstrFile, const char *filter, const char *title, const char *defExt, unsigned int flags = 0, unsigned int filterIndex = 0);
void GetAppDataString(char *buffer, size_t len);

#endif	//FILEUTILITIES_H
