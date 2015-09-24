#ifndef TI81HW_H
#define TI81HW_H
#include "../core/core.hpp"
#include "ti_stdint.hpp"

#define NumElm(array) (sizeof (array) / sizeof ((array)[0]))

int device_init_81(CPU_t*);
int memory_init_81(memc *);

#endif 
