
#define _GNU_SOURCE
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct dns_req_manager_t
{
    struct sigevent event;
    struct gaicb **reqs;
};


void dns_cb(union sigval arg)
{
    int ret = 0;
    struct addrinfo *res = NULL;
    struct sockaddr_in addr;
    struct dns_req_manager_t *req_manager = arg.sival_ptr;

    printf("DNS translation success.\n");
    ret = gai_error(req_manager->reqs[0]);
    if (ret == 0)
    {
        res = req_manager->reqs[0]->ar_result;
        memcpy(&addr, res->ai_addr, sizeof(addr));
        printf("server ip: %s\n", inet_ntoa(addr.sin_addr));

    }
    else
    {
        printf("DNS translation error:%s\n", gai_strerror(ret));
    }
    free((char *)(req_manager->reqs[0]->ar_name));
    freeaddrinfo(req_manager->reqs[0]->ar_result);
    free(req_manager->reqs[0]);
    free(req_manager->reqs);
    free(req_manager);
}

int get_sockaddr_info_a(char *node)
{
    int ret          = -1;
    struct dns_req_manager_t *req_manager = NULL;

    if (!node)
    {
        printf("node parameter is null.\n");
        return -1;
    }

    req_manager = (struct dns_req_manager_t *)malloc(sizeof(*req_manager));
    if (!req_manager)
    {
        return -1;
    }
    memset(req_manager, 0, sizeof(*req_manager));

    req_manager->reqs = realloc(req_manager->reqs, 1*sizeof(req_manager->reqs[0]));
    req_manager->reqs[0] = calloc(1, sizeof(*req_manager->reqs[0]));
    req_manager->reqs[0]->ar_name = strndup(node, 68); /*domain name max size is 67 bytes*/

     memset(&req_manager->event, 0, sizeof(req_manager->event));
     req_manager->event.sigev_notify = SIGEV_THREAD;
     req_manager->event.sigev_notify_function = dns_cb;
     req_manager->event.sigev_value.sival_ptr = req_manager;
    
    ret = getaddrinfo_a(GAI_NOWAIT, req_manager->reqs, 1, &req_manager->event); 
    if (ret != 0)
    {
        printf("DNS translation fail.\n");
    }

    return ret;
}


int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        return -1;
    }
    
    get_sockaddr_info_a(argv[1]);
    /*
     *pause();
     */
    sleep(7);

    return 0;
}
