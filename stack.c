#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

//add -finstrument-functions

FILE *callgraph_fp = NULL;
//int g_count;
//#define DUMP(desp , func, call)  fprintf(fp ,"%s:%p:%p\n", desp , func , __builtin_return_address(1))
//
void __attribute__((__no_instrument_function__)) dump(char *desp , void *func)
{
    fprintf(callgraph_fp , "%s:%p\n", desp, func);
    fflush(callgraph_fp);
}

/* Constructor and Destructor Prototypes */
void main_constructor( void ) __attribute__ ((no_instrument_function, constructor));
void main_destructor( void ) __attribute__ ((no_instrument_function, destructor));
    /* Output trace file pointer */

void main_constructor(void)
{
    callgraph_fp = fopen("/data/app/log/trace.txt", "w");
    if (callgraph_fp == NULL) 
        exit(-1);
    //g_count = 0;
}

void main_destructor(void)
{
    fclose(callgraph_fp);
}

void __attribute__((__no_instrument_function__))
__cyg_profile_func_enter(void *this_func, void *call_site)
{
    dump("enter", this_func);
}

void __attribute__((__no_instrument_function__))
__cyg_profile_func_exit(void *this_func, void *call_site)
{
    dump("leave", this_func);
}

#ifdef __cplusplus
}
#endif

