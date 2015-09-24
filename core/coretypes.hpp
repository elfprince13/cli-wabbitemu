#ifndef CORETYPES_H_
#define CORETYPES_H_

#define NumElm(array) (sizeof (array) / sizeof ((array)[0]))

#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstddef>
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <strings.h>
#include <ctime>

#define ARRAYSIZE(z) (sizeof(z)/sizeof((z)[0]))

typedef struct _RECT {
	int64_t left;
	int64_t top;
	int64_t right;
	int64_t bottom;
} RECT;


#endif /*CORETYPES_H_*/
