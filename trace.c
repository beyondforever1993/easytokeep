#ifdef __cplusplus
extern "C" {
#endif

#define _GNU_SOURCE	
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <dirent.h>
#include <errno.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/stat.h>

#define gettid_self()   syscall(__NR_gettid)

extern char *__progname;

static __thread int s_log_fd = -1;
static __thread int s_space_cnt = 0;
static __thread long int thread_id = 0;
static int s_thread_index = 0;
static pthread_key_t s_fd_key;
static pthread_once_t s_fd_key_once = PTHREAD_ONCE_INIT;

void __cyg_profile_func_enter(void *this_fn, void *call_site) __attribute__ ((no_instrument_function));
void __cyg_profile_func_exit(void *this_fn, void *call_site) __attribute__ ((no_instrument_function));

/* Constructor and Destructor Prototypes */
void main_constructor(void) __attribute__ ((no_instrument_function, constructor));
void main_destructor(void) __attribute__ ((no_instrument_function, destructor));
static void segment_handler(int sig_num) __attribute__((no_instrument_function));
static void fd_destroy(void *nouse) __attribute__((no_instrument_function));
static void thread_destructor() __attribute__((no_instrument_function));
static void close_all_fd() __attribute__((no_instrument_function));

static void close_all_fd()
{
    char pid_dir[64] = {0};
    char target_file[128] = {0};
    char strerr[128] = {0};
    DIR *dirp = NULL; 
    struct dirent entry;
    struct dirent *result = NULL;

    snprintf(pid_dir, sizeof(pid_dir)-1, "/proc/%d/fd", getpid());

    if ((dirp = opendir(pid_dir)) == NULL)
    {
        strerror_r(errno, strerr, sizeof(strerr)-1);
        printf("Open directory error [%s]\n", strerr);
        return;
    }

    while ((readdir_r(dirp, &entry, &result) == 0) 
            && result != NULL)
    {
        if (entry.d_type == DT_LNK && atoi(entry.d_name) > 2)
        {
            snprintf(pid_dir, sizeof(pid_dir)-1, "/proc/%d/fd/%s", 
                    getpid(), entry.d_name);
            readlink(pid_dir, target_file, sizeof(target_file));
            printf("fd: %s, target file: %s\n", entry.d_name, target_file);
            memset(target_file, 0, sizeof(target_file));
            if (strstr(entry.d_name, __progname))
            {
                fsync(atoi(entry.d_name));
                close(atoi(entry.d_name));
            }
        }
    }

    closedir(dirp);
}

static void segment_handler(int sig_num)
{
    if (sig_num == SIGSEGV) 
    {
        close_all_fd();

        exit(EXIT_FAILURE);
    }
}

void main_constructor(void)
{
    struct  sigaction   sigsegv;

    /*Create trace directory*/
    mkdir("trace", 0775);

    memset(&sigsegv, 0, sizeof(sigsegv));
    sigsegv.sa_handler = segment_handler;
    sigemptyset(&sigsegv.sa_mask);
    sigsegv.sa_flags = SA_RESTART;

    sigaction(SIGSEGV, &sigsegv, NULL);
}

void main_destructor(void)
{
    close_all_fd();
}

static void fd_destroy(void *nouse)
{
    if (s_log_fd > 0)
    {
        fsync(s_log_fd);
        close(s_log_fd);
        s_log_fd = -1;
    }
}

static void thread_destructor()
{
    pthread_key_create(&s_fd_key, fd_destroy);
}

void __cyg_profile_func_enter(void *this_fn, void *call_site)
{
    char logfile[128] = {0};
    Dl_info DLInfo;

    if (s_log_fd < 0)
    {
        pthread_once(&s_fd_key_once, thread_destructor);
        pthread_setspecific(s_fd_key, &s_log_fd);

        thread_id = gettid_self();
        snprintf(logfile, sizeof(logfile)-1, "trace/%s_%ld_%d", 
                __progname, thread_id, s_thread_index++);

        if ((s_log_fd = open(logfile, 
                        O_CREAT|O_RDWR|O_TRUNC|O_LARGEFILE, 0666)) < 0)
            _exit(EXIT_FAILURE);
    }

    dladdr(this_fn, &DLInfo);
    dprintf(s_log_fd, "%s (%ld): %-*s-> %s\n", 
            __progname, thread_id, s_space_cnt, "", DLInfo.dli_sname);
    ++s_space_cnt;
}

void __cyg_profile_func_exit(void *this_fn, void *call_site)
{
    Dl_info DLInfo;

    dladdr(this_fn, &DLInfo);
    --s_space_cnt;
    dprintf(s_log_fd, "%s (%ld): %-*s<- %s\n", 
            __progname, thread_id, s_space_cnt, "", DLInfo.dli_sname);
}

#ifdef __cplusplus
}
#endif

