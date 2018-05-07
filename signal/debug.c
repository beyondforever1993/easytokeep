#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <execinfo.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "debug.h"
#define SIZE 100
#define DEBUG_FILE "record_bt"


sig_info_t sigCatch[] = 
{
    {1, "SIGHUP"}, 
    {2, "SIGINT"}, 
    {3, "SIGQUIT"}, 
    {4, "SIGILL"},
    {6, "SIGABRT"}, 
    {8, "SIGFPE"}, 
    {11, "SIGSEGV"},
    {14, "SIGALRM"},
    {15, "SIGTERM"},
    {10, "SIGUSR1"},
    {12, "SIGUSR2"}
};

void blackbox_handler(int sig)
{
    int fd = -1;
    char buff[128];
    int nptrs;
    void *buffer[100];

    fd = open(DEBUG_FILE,  O_CREAT|O_RDWR|O_APPEND, 0666);
    if (fd < 0)
        return;

    snprintf(buff, sizeof(buff)-1, "Enter blackbox_handler: SIG name is %s, SIG num is %d\n", strsignal(sig), sig);
    write(fd, buff, strlen(buff));
    nptrs = backtrace(buffer, SIZE);
    snprintf(buff, sizeof(buff)-1, "backtrace() returned %d address\n", nptrs);
    write(fd, buff, strlen(buff));
    backtrace_symbols_fd(buffer, nptrs, fd);
    close(fd);

    _exit(EXIT_SUCCESS);
}

void registe_sig_handler()
{
    int i;
    struct  sigaction   sa;
    memset(&sa, 0, sizeof(sa));

    sa.sa_handler = blackbox_handler;
    sigemptyset(&sa.sa_mask);

    sa.sa_flags = 0;

    for (i = 0; i < sizeof(sigCatch)/sizeof(sig_info_t); i++)
    {
        if(sigaction(sigCatch[i].signum, &sa, NULL) < 0)
        {
            return;
        }
    }

}

void exit_cb()
{
    int  nptrs;
    void *buffer[100];
    struct stat fd_stat;

    int fd = open(DEBUG_FILE,  O_CREAT|O_RDWR|O_APPEND, 0666);

    fstat(fd, &fd_stat);
    if (((fd_stat.st_size * 1.0) / (1024*1024)) > 10)
    {
        ftruncate(fd, 0);  /* truncate the log file to empty file */
        lseek(fd, 0, SEEK_SET);
    }

    nptrs = backtrace(buffer, SIZE);
    backtrace_symbols_fd(buffer, nptrs, fd);
    close(fd);
}

