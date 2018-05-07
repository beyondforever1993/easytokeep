#ifndef __DBEUG_PROGRAM_H__
#define __DBEUG_PROGRAM_H__
#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <execinfo.h>

typedef struct sig_info
{
    int     signum;
    char    signame[20];
} sig_info_t;

extern void blackbox_handler(int sig);
extern void registe_sig_handler();
extern void exit_cb();

#ifdef __cplusplus
}
#endif
#endif
