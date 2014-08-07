#ifndef _DONTWORRYDEBUG_H_
#define _DONTWORRYDEBUG_H_
//#define PRINT_PARSE_DEBUG

#include <stdio.h>

// macro pour le débug
#ifdef PRINT_PARSE_DEBUG
#define DBG(x) printf(x)
#define DBG2(x,y) printf(x,y)
#else
#define DBG(x) 
#define DBG2(x,y) 
#endif

#endif
