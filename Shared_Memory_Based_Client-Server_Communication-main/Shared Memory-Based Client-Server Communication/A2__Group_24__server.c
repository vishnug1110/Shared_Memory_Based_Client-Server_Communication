#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>
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


#define MAX_NAME_LENGTH 20
#define MAX_CLIENT_COUNT 50
#define MAX_KEY_LENGTH 15
#define SHM_SIZE 4096

int total_requests_served = 0;
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
struct connection_request
{
    pthread_mutex_t mutex;
    char name[100];
    char key[10];
    bool key_recieved;
    bool key_sent;
    enum request_state cst;

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

}; // this structure forms the comm channel
   // intializing mutex
struct thread_data
{
    char glo_key[10];
    bool is_deregistered;
    int request_count;
};

void *runner(struct thread_data *data) // this is thread function which performs mathematical calculations
{

    int fd = shm_open(data->glo_key, O_CREAT | O_RDWR, 0666);

    ftruncate(fd, 4096);

    struct cl_req *ptr2 = (struct cl_req *)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    PRINT_INFO("<---------COMM CHANNEL CREATED---------->");
    PRINT_INFO("CLIENT CONNECTED TO COMM CHANNEL");
    pthread_mutex_init(&(ptr2->clmutex), NULL);
    while (1)
    {
        PRINT_INFO("REQUEST COUNT FOR CLIENT WITH KEY %s is %d\n", data->glo_key, data->request_count);

        while (1)
        {
            pthread_mutex_lock(&(ptr2->clmutex));
            if (ptr2->rst == request_buffer_empty)
            {
                pthread_mutex_unlock(&(ptr2->clmutex));
                sleep(1);
            }
            else
            {
                PRINT_INFO("COMM CHANNEL SERVICE REQUEST RECEIVED");
                break;
            }
        }

        if (ptr2->rst == deregistration)
        {
            PRINT_INFO("UNREGISER REQUEST RECEIVED");
            data->is_deregistered = 1;

            break;
        }
        if (ptr2->rst == end_request)
        {
            ptr2->rst = request_buffer_empty;
            break;
        }

        while (ptr2->rst != request_params_sent)
            ;
        ptr2->rst = request_inuse;

        if (ptr2->d.cop == arth)
        {
            int ok = 0;
            int n1 = ptr2->d.crd.ath.n1;
            int n2 = ptr2->d.crd.ath.n2;
            int op = ptr2->d.crd.ath.op;
            int ans;
            if (op == 1)
            {
                ans = n1 + n2;
            }
            else if (op == 2)
            {
                ans = n1 - n2;
            }
            else if (op == 3)
            {
                ans = n1 / n2;
            }
            else if (op == 4)
            {
                ans = n1 * n2;
            }

            ptr2->rp.ActionSpecificResponse = ans;
            ptr2->rp.ClientResponseSeqNo = 1;
            ptr2->rp.response_code = 0;
            ptr2->rp.ServerResponseSeqNo = total_requests_served;
            data->request_count++;
            total_requests_served++;
            PRINT_INFO("COMM CHANNEL REQUEST RESPONSE SUCCESSFUL");
        }
        else if (ptr2->d.cop == oddeven)
        {
            int ok = 0;
            int n1 = ptr2->d.crd.e.n1;
            if (n1 % 2)
            {
                ok = 0;
            }
            else
            {
                ok = 1;
            }

            ptr2->rp.ActionSpecificResponse = ok;
            ptr2->rp.ClientResponseSeqNo = 1;
            ptr2->rp.response_code = 0;
            ptr2->rp.ServerResponseSeqNo = total_requests_served;
            data->request_count++;
            total_requests_served++;
            PRINT_INFO("COMM CHANNEL REQUEST RESPONSE SUCCESSFUL");
        }
        else if (ptr2->d.cop == prime)
        {
            int ok = 1;
            int n1 = ptr2->d.crd.p.n1;

            for (int i = 2; i < n1; i++)
            {
                if (n1 % i == 0)
                {
                    ok = 0;
                }
            }
            ptr2->rp.ActionSpecificResponse = ok;
            ptr2->rp.ClientResponseSeqNo = 1;
            ptr2->rp.response_code = 0;
            ptr2->rp.ServerResponseSeqNo = total_requests_served;
            data->request_count++;
            total_requests_served++;
            PRINT_INFO("COMM CHANNEL REQUEST RESPONSE SUCCESSFUL");
        }

        // is negative
        else if (ptr2->d.cop == isNegative)
        {
            int ok = 0;
            int n1 = ptr2->d.crd.neg.n1;
            if (n1 < 0)
            {
                ok = 1;
                ptr2->rp.response_code = 500;
            }
            else
            {
                ptr2->rp.response_code = 0;
            }
            ptr2->rp.ActionSpecificResponse = ok;
            ptr2->rp.ClientResponseSeqNo = 1;
            ptr2->rp.ServerResponseSeqNo = total_requests_served;
            data->request_count++;
            total_requests_served++;
            PRINT_INFO("COMM CHANNEL REQUEST RESPONSE SUCCESSFUL");
        }

        ptr2->rst = request_serverd;
        sleep(1);
        pthread_mutex_unlock(&(ptr2->clmutex));
    }
    ptr2->rst = request_serverd;
    

    if (shm_unlink(data->glo_key) == -1)
    {
        perror("shm_unlink");
        exit(EXIT_FAILURE);
    }
    close(fd);

    PRINT_INFO("COMM CHANNEL CLEANED UP");

    sleep(1);
    pthread_mutex_unlock(&(ptr2->clmutex));
    // printf("exiting thread\n");
    pthread_exit(NULL);

    return NULL;
}
// this function generates a unique key of length 10
char *generate_key(char key[10])
{

    static const char alphabet[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    const int alphabet_size = sizeof(alphabet) - 1;

    srand(time(NULL));
    for (int i = 0; i < 9; i++)
    {
        key[i] = alphabet[rand() % alphabet_size];
    }
    key[9] = '\0';

    return key;
}

void sigint_handler(int signum, void *arg)
{
    const char *shm_name = "connect-channel";
    void *shm_ptr = NULL;

    int shm_fd = shm_open(shm_name, O_RDWR, 0666);
    if (shm_fd == -1)
    {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }
    shm_ptr = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    if (munmap(shm_ptr, SHM_SIZE) == -1)
    {
        perror("munmap");
        exit(EXIT_FAILURE);
    }

    if (shm_unlink(shm_name) == -1)
    {
        perror("shm_unlink");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}

int main()
{

    signal(SIGINT, sigint_handler);

    const int size = 4096;
    const char *name = "connect-channel";
    struct connection_request *ptr;
    int fd = shm_open(name, O_CREAT | O_RDWR, 0666);

    ftruncate(fd, 4096);

    ptr = (struct connection_request *)mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    PRINT_INFO("SERVER INITIATED AND CONNECT CHANNEL FORMED");

    int server_running = 1;

    pthread_mutex_init(&(ptr->mutex), NULL);

    // start

    char client_names[MAX_CLIENT_COUNT][MAX_NAME_LENGTH];
    char client_keys[MAX_CLIENT_COUNT][MAX_KEY_LENGTH];
    struct thread_data *client_data[MAX_CLIENT_COUNT];
    int client_count = 0;

    // end

    while (server_running) // this ensures that the server is always running
    {
        for (int i = 0; i < client_count; i++)
        {
            PRINT_INFO("%s ", client_names[i]);
        }

        PRINT_INFO("TOTAL REQUESTS SERVED %d", total_requests_served);

        while (1)
        {

            pthread_mutex_lock(&(ptr->mutex)); // locking the server when client sends the reg request

            if (ptr->cst == request_buffer_empty)
            {

                // logic for deregisteration starts
                ptr->cst = deregistration;
                int i = 0;
                while (i < client_count)
                {
                    if (client_data[i]->is_deregistered == 1)
                    {
                        char tempkey[MAX_KEY_LENGTH];
                        strcpy(tempkey, client_data[i]->glo_key);

                        for (int j = i; j < client_count - 1; j++)
                        {
                            strcpy(client_names[j], client_names[j + 1]);
                            strcpy(client_keys[j], client_keys[j + 1]);

                            client_data[j] = client_data[j + 1];
                        }

                        client_count--;
                        PRINT_INFO("CLIENT WITH KEY %s DEREGISTERED", tempkey);
                    }

                    else
                    {
                        i++;
                    }
                }
                // logic ends

                pthread_mutex_unlock(&(ptr->mutex));
                ptr->cst = request_buffer_empty;
                sleep(1);
            }
            else
            {
                ptr->cst = request_inuse;
                break;
            }
        }
        while (ptr->cst != name_received)
            ;
        ptr->cst = request_inuse;
        PRINT_INFO("REQUEST RECEIVED FROM CLIENT %s ON CONNECT CHANNEL\n", ptr->name);

        // check for unique names
        bool key_exists = 0;
        if (client_count == MAX_CLIENT_COUNT)
        {
            PRINT_INFO("Maximum client limit reached\n");
            continue;
        }

        struct thread_data *temp_data = (struct thread_data *)malloc(sizeof(struct thread_data));

        for (int i = 0; i < client_count; i++)
        {
            if (strcmp(ptr->name, client_names[i]) == 0)
            {
                PRINT_INFO("Name already exists in connection channel\n");
                key_exists = 1;
                strcpy(ptr->key, client_keys[i]);

                break;
            }
        }

        if (!key_exists)
        {
            strcpy(client_names[client_count], ptr->name);
            generate_key(ptr->key);
            strcpy(client_keys[client_count], ptr->key);
            strcpy(temp_data->glo_key, ptr->key);
            temp_data->request_count = 0;
            temp_data->is_deregistered = 0;
            client_count++;
        }

        client_data[client_count - 1] = temp_data;
        ptr->key_sent = 1;
        PRINT_INFO("CONNECT CHANNEL REGISTER REQUEST SUCCESSFUL");
        PRINT_INFO("CLIENT WITH NAME %s HAS KEY %s", ptr->name, ptr->key);
        pthread_t t;

        if (pthread_create(&t, NULL, (void *)&runner, temp_data) == -1) // creating the worker thread
        {
            PRINT_ERR_EXIT("Error in thread creation");
        }

        ptr->cst = deregistration;
        int i = 0;
        while (i < client_count)
        {
            if (client_data[i]->is_deregistered == 1)
            {
                char tempkey[MAX_KEY_LENGTH];
                strcpy(tempkey, client_data[i]->glo_key);

                for (int j = i; j < client_count - 1; j++)
                {
                    strcpy(client_names[j], client_names[j + 1]);
                    strcpy(client_keys[j], client_keys[j + 1]);
                    client_data[j] = client_data[j + 1];
                }
                client_count--;
                PRINT_INFO("CLIENT WITH KEY %s DEREGISTERED", tempkey);
            }

            else
            {
                i++;
            }
        }
        // logic ends
        pthread_mutex_unlock(&(ptr->mutex));
        ptr->cst = request_buffer_empty;
    }
}
