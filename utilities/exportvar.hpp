#ifndef EXPORTVAR_H
#define EXPORTVAR_H

#include "../hardware/link.hpp"

typedef struct {
	FILE* stream;
	unsigned int pnt;
	unsigned int size;
	unsigned char *data;
	unsigned char *name;
	bool read;
	bool write;
	bool bin;
	int eof;
} MFILE;

MFILE *ExportVar(LPCALC, char *, symbol83P_t *);
MFILE *ExportApp(LPCALC, char *, apphdr_t *);
MFILE *ExportRom(char *lpszFile, LPCALC lpCalc);
MFILE * ExportOS(char *lpszFile, unsigned char *buffer, int size);
MFILE *mopen(const char *filename, const char * mode);
int mclose(MFILE *);
int meof(MFILE *);
int mgetc(MFILE *);
int mputc(int, MFILE *);
int msize(MFILE *);

#endif
