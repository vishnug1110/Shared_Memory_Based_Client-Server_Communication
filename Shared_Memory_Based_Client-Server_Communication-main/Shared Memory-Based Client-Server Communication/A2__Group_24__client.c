#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdbool.h>
#include <pthread.h>
#include "CSF372.h"


/*

 Group 24
 Group members:-    Devashish Deshpande - 2020A3PS2207H
                    Shreenidhi Joisa    - 2020A3PS2136H
                 	Kedar Nandkhedkar   - 2020A3PS0481H
                	Prakhar Gupta       - 2020A8PS0697H
                  	Faizal Shaikh       - 2020AAPS2107H
                 	Aadyan Choudhary    - 2021A4PS2846H
                 	Dhruv Jain - 		- 2020A3PS2214H
                 	Chinmayee Selukar   - 2020A8PS0735H
*/


enum request_state
{
    request_buffer_empty,
    request_inuse,
    request_serverd,
    request_params_sent,
    end_request,
    name_received,
    deregistration

};
enum authentication
{
    authenciation_passed,
    authenciation_failed,
    authenciation_inprogress
};
struct connection_request
{
    pthread_mutex_t mutex;
    char name[100];
    char key[10];
    bool key_recieved;
    bool key_sent;
    enum request_state cst;
    enum authentication auth;

}; // connectinon request sent by client to perform registration
struct arithmatic
{
    int n1;
    int n2;
    int op;
};

struct even_odd
{
    int n1;
};

struct isPrime
{
    int n1;
};

struct isNegative
{
    int n1;
};

union client_request_data
{
    struct arithmatic ath;
    struct even_odd e;
    struct isPrime p;
    struct isNegative neg;
};

enum client_op
{
    arth,
    oddeven,
    prime,
    isNegative
};

struct client_request
{

    enum client_op cop;
    union client_request_data crd;
};

struct client_response
{
    int response_code;
    int ClientResponseSeqNo;
    int ServerResponseSeqNo;
    int ActionSpecificResponse;
};

struct cl_req
{

    pthread_mutex_t clmutex;

    enum request_state rst;
    struct client_request d;
    struct client_response rp;
};
int main()
{

    const char *name = "connect-channel";
    int fd;

    struct connection_request *ptr;

    fd = shm_open(name, O_RDWR, 0666);
    if (fd == -1)
    {
        PRINT_ERR_EXIT("SHM OPEN FAILED");
    }
    ptr = (struct connection_request *)mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED)
    {
        PRINT_ERR_EXIT("mmap");
    }

    while (ptr->cst != request_buffer_empty)
        ;
    ptr->cst = request_inuse;
    PRINT_INFO("REGISTER REQUEST MADE FROM CLIENT");

    scanf("%s", ptr->name);
    ptr->cst = name_received;

    while (ptr->key_sent == 0)
    {
    }
    // waiting for the unique key

    char local_key[10];
    strcpy(local_key, ptr->key);

    ptr->key_sent = 0;

    PRINT_INFO("CLIENT SUCCESSFULLY REGISTERED ON CONNECT CHANNEL");
    PRINT_INFO("CLIENT WITH NAME %s HAS KEY %s", ptr->name, ptr->key);

    sleep(2);

    int fd2 = shm_open(local_key, O_RDWR, 0666);

    struct cl_req *ptr2 = (struct cl_req *)mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd2, 0);

    ptr2->rst = request_buffer_empty;
    PRINT_INFO("CLIENT CONNECTED TO COMM CHANNEL");
    while (1)
    {
        int action_type;
        PRINT_INFO("PRESS 1 TO DEREGISTER AND 2 TO SEND REQUEST TO COMM CHANNEL \n");
        scanf("%d", &action_type);
        if (action_type == 1)
        {
            // deregistration
            ptr2->rst = deregistration;
            munmap(ptr2, 4096);

            close(fd);
            PRINT_INFO("COMM CHANNEL CLEANED UP");

            break;
        }
        else if (action_type == 2)
        {
            int operation_type;
            PRINT_INFO("To send arithmetic request press 1 \n");
            PRINT_INFO("To send even odd request press 2 \n");
            PRINT_INFO("To send prime request press 3 \n");
            PRINT_INFO("TO SEND ISNEGATIVE REQUEST PRESS 4\n");
            scanf("%d", &operation_type);
            if (operation_type == 1)
            {
                ptr2->rst = request_inuse;
                ptr2->d.cop = arth;
                int n1, n2;
                int op;
                PRINT_INFO("Enter 2 integers \n");
                scanf("%d %d", &n1, &n2);
                PRINT_INFO("Enter 1 to perform addition\n");
                PRINT_INFO("Enter 2 to perform substraction \n");
                PRINT_INFO("Enter 3 to perform divison \n");
                PRINT_INFO("Enter 4 to perform multiplication \n");
                scanf("%d", &op);

                ptr2->d.crd.ath.n1 = n1;
                ptr2->d.crd.ath.n2 = n2;
                ptr2->d.crd.ath.op = op;
                ptr2->rst = request_params_sent;
                PRINT_INFO("REQUEST BY CLIENT SENT TO COMM CHANNEL");
            }
            else if (operation_type == 2)
            {
                ptr2->rst = request_inuse;
                ptr2->d.cop = oddeven;
                int n1;
                PRINT_INFO("Enter the number for the request \n");
                scanf(" %d", &n1);
                ptr2->d.crd.e.n1 = n1;
                ptr2->rst = request_params_sent;
                PRINT_INFO("REQUEST BY CLIENT SENT TO COMM CHANNEL");
            }
            else if (operation_type == 3)
            {
                ptr2->rst = request_inuse;
                ptr2->d.cop = prime;
                int n1;
                PRINT_INFO("Enter the number for the request \n");
                scanf("%d", &n1);
                ptr2->d.crd.p.n1 = n1;
                ptr2->rst = request_params_sent;
                PRINT_INFO("REQUEST BY CLIENT SENT TO COMM CHANNEL");
            }

            else if (operation_type == 4)
            {
                ptr2->rst = request_inuse;
                ptr2->d.cop = isNegative;
                int n1;
                PRINT_INFO("Enter the number for the request \n");
                scanf("%d", &n1);
                ptr2->d.crd.neg.n1 = n1;
                ptr2->rst = request_params_sent;
                PRINT_INFO("REQUEST BY CLIENT SENT TO COMM CHANNEL");
            }

            while (ptr2->rst != request_serverd)
            {
            }
            PRINT_INFO("RESPONSE FROM COMM CHANNEL TO CLIENT IS %d\n", ptr2->rp.ActionSpecificResponse);
            ptr2->rst = request_buffer_empty;
        }
        else
        {
            ptr2->rst = end_request;
            munmap(ptr2, 4096);

            close(fd);
            PRINT_INFO("COMM CHANNEL CLEANED UP");
            break;
        }
    }
}
