//including necessary libraries
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

//definitions
#define TOTAL_QUEUES 10
#define ROW_SIZE 10

//structure definitions
typedef struct customer
{
    int arrival_time;
    int custId;
} customer;

typedef struct customerQueue_struct
{
    customer *client;
} customerQueue;

typedef struct row_struct
{
    pthread_mutex_t mutex;
} row;

typedef struct pthread_args_struct
{
    int tid;
    int row_id;
    char *seller_type;
    customerQueue *cq;
} pthread_args;

// seat status enum
enum seat_state
{
    AVAILABLE,
    SOLD,
    PROCESSING
};

typedef struct seat_struct
{
    int id;
    int counter;
    enum seat_state state;
    customer *cust;
    pthread_args *p_args;
} seat;

struct seat_manager_struct
{
    int free_seats;
    seat *h_seat;
    seat *m_seat;
    seat *l_seat;
} seat_manager;

// initializing parameters
customerQueue *cQ;
row tickets[10];
static int serve_time = 0;
int pt = 0;

int h_customers = 0;
int m_customers = 0;
int l_customers = 0;

int thr_H = 0;
int tt_H = 0;
int rt_H = 0;
int total_h_customers = 0;

int thr_L1 = 0;
int tt_L1 = 0;
int rt_L1 = 0;
int total_l1_customers = 0;

int thr_L2 = 0;
int tt_L2 = 0;
int rt_L2 = 0;
int total_l2_customers = 0;

int thr_L3 = 0;
int tt_L3 = 0;
int rt_L3 = 0;
int total_l3_customers = 0;

int thr_L4 = 0;
int tt_L4 = 0;
int rt_L4 = 0;
int total_l4_customers = 0;

int thr_L5 = 0;
int tt_L5 = 0;
int rt_L5 = 0;
int total_l5_customers = 0;

int thr_L6 = 0;
int tt_L6 = 0;
int rt_L6 = 0;
int total_l6_customers = 0;

int thr_M1 = 0;
int tt_M1 = 0;
int rt_M1 = 0;
int total_m1_customers = 0;

int thr_M2 = 0;
int tt_M2 = 0;
int rt_M2 = 0;
int total_m2_customers = 0;

int thr_M3 = 0;
int tt_M3 = 0;
int rt_M3 = 0;
int total_m3_customers = 0;

// total number of customers
int N;

// initializing thread parameters
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t seat_lock = PTHREAD_MUTEX_INITIALIZER; // The lock for the seat manager struct

// generating 100 seats
seat theater[100];

// prints theater
void show_theater()
{
    int i, j, idx;
    for (i = 0; i < 10; i++)
    {
        printf("ROW %02d |", i);
        for (j = 0; j < 10; j++)
        {
            idx = i * 10 + j;
            if (theater[idx].state == AVAILABLE)
            {
                printf(" S%02d: ********* |", theater[idx].id);
            }
            else
            {
                if (theater[idx].cust == NULL)
                {
                    printf("error found NULL\n");
                }
                else
                {
                    printf(" S%02d: %s%d=%02d=C%02d |", theater[idx].id, theater[idx].p_args->seller_type, theater[idx].p_args->tid, theater[idx].counter, theater[idx].cust->custId);
                }
            }
        }
        printf("\n");
    }
    printf("\n");
}

// initializes theater
void initialize_theater()
{
    int i;
    for (i = 0; i < 100; i++)
    {
        theater[i].state = AVAILABLE;
        theater[i].id = i;
        theater[i].cust = NULL;
        theater[i].p_args = NULL;
    }
}

// initializes seat manager
void initialize_seat_manager()
{
    seat_manager.h_seat = &theater[0];
    seat_manager.m_seat = &theater[40];
    seat_manager.l_seat = &theater[90];
    seat_manager.free_seats = 100;
}

// goes to next h seat
void h_next_seat()
{
    seat *tmp_seat = seat_manager.h_seat;
    while (tmp_seat->state != AVAILABLE && seat_manager.free_seats > 0)
    {
        tmp_seat++;
    }
    seat_manager.h_seat = tmp_seat;
}

// goes to next m seat
void m_next_seat()
{
    seat *tmp_seat = seat_manager.m_seat;
    while (tmp_seat->state != AVAILABLE && seat_manager.free_seats > 0)
    {
        // if seat id is 99, 89, 79, etc subtract 19 to move to previous row

        switch (tmp_seat->id)
        {

        case 59:
            tmp_seat = &theater[30];
            continue;

        case 39:
            tmp_seat = &theater[60];
            continue;

        case 69:
            tmp_seat = &theater[20];
            continue;

        case 29:
            tmp_seat = &theater[70];
            continue;

        case 79:
            tmp_seat = &theater[10];
            continue;

        case 19:
            tmp_seat = &theater[80];
            continue;

        case 89:
            tmp_seat = &theater[0];
            continue;

        case 9:
            tmp_seat = &theater[90];
            continue;

        default:
            tmp_seat++;
        }
    }
    seat_manager.m_seat = tmp_seat;
}

// goes to next l seat
void l_next_seat()
{
    seat *tmp_seat = seat_manager.l_seat;

    while (tmp_seat->state != AVAILABLE && seat_manager.free_seats > 0)
    {
        if (tmp_seat->id % 10 == 9 && tmp_seat->id > 9)
        {
            tmp_seat = &theater[tmp_seat->id - 19];
            continue;
        }

        // try the next seat
        tmp_seat++;
    }
    seat_manager.l_seat = tmp_seat;
}

// sells h seat
seat *get_h_seat_to_sell()
{
    seat *allocated_seat_to_sell;
    pthread_mutex_lock(&seat_lock);
    if (seat_manager.free_seats > 0)
    {
        // If the seat pointed to by the seat_manager is not free, get a free seat.
        if (seat_manager.h_seat->state != AVAILABLE)
            h_next_seat();
        allocated_seat_to_sell = seat_manager.h_seat;
        allocated_seat_to_sell->state = PROCESSING;
        seat_manager.free_seats--;
        h_customers++;
    }
    else
    {
        allocated_seat_to_sell = NULL;
    }
    pthread_mutex_unlock(&seat_lock);
    return allocated_seat_to_sell;
}

// sells m seat
seat *get_m_seat_to_sell()
{
    seat *allocated_seat_to_sell;
    pthread_mutex_lock(&seat_lock);
    if (seat_manager.free_seats > 0)
    {
        // If the seat pointed to by the seat_manager is not free, get a free seat.
        if (seat_manager.m_seat->state != AVAILABLE)
            m_next_seat();
        allocated_seat_to_sell = seat_manager.m_seat;
        allocated_seat_to_sell->state = PROCESSING;
        seat_manager.free_seats--;
        m_customers++;
    }
    else
    {
        allocated_seat_to_sell = NULL;
    }
    pthread_mutex_unlock(&seat_lock);
    return allocated_seat_to_sell;
}

// sells l seat
seat *get_l_seat_to_sell()
{
    seat *allocated_seat_to_sell;
    pthread_mutex_lock(&seat_lock);
    if (seat_manager.free_seats > 0)
    {
        // If the seat pointed to by the seat_manager is not free, get a free seat.
        if (seat_manager.l_seat->state != AVAILABLE)
            l_next_seat();
        allocated_seat_to_sell = seat_manager.l_seat;
        allocated_seat_to_sell->state = PROCESSING;
        seat_manager.free_seats--;
        l_customers++;
    }
    else
    {
        allocated_seat_to_sell = NULL;
    }
    pthread_mutex_unlock(&seat_lock);
    return allocated_seat_to_sell;
}

// seller thread to serve one quanta
void *seller_thread(pthread_args *pargs)
{
    int customer_idx = 0;
    int all_done = 0;
    int process_time = 0;
    int done_time = 0;
    seat *sts;
    customer *nc;

    while (1)
    {
        // clock barrier
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&cond, &mutex);
        pthread_mutex_unlock(&mutex);

        // Check to see if we are in the middle of a sale and continue
        if (serve_time < done_time)
        {
            continue;
        }

        // Skip this turn if next customer has not arrived yet.
        if (pargs->cq->client[customer_idx].arrival_time > serve_time)
        {
            continue;
        }

        // get the customer if we are not done yet
        if (customer_idx < N)
        {
            nc = &(pargs->cq->client[customer_idx++]);
        }
        else
        {
            if (all_done == 0)
            {
                all_done = 1;
            }
            continue;
        }

        // We have a customer to process now, get seat and random wait time depending on the seller type
        if (strcmp(pargs->seller_type, "H") == 0)
        {
            sts = get_h_seat_to_sell();
            process_time = 1 + (rand() % 1);
        }
        else if (strcmp(pargs->seller_type, "M") == 0)
        {
            sts = get_m_seat_to_sell();
            process_time = 2 + (rand() % 2);
        }
        else
        {
            sts = get_l_seat_to_sell();
            process_time = 4 + (rand() % 3);
        };

        if (sts == NULL)
        {
            //printf(" THEATER SOLD OUT!!!\n");
            break;
        }

        // Finish processing the customer
        done_time = serve_time + process_time;
        sts->cust = nc;
        sts->p_args = pargs;
        sts->state = SOLD;
        if (strcmp(pargs->seller_type, "H") == 0)
        {
            total_h_customers++;
            thr_H = thr_H + process_time;
            tt_H = tt_H + done_time + pargs->cq->client[customer_idx].arrival_time;
            rt_H = rt_H + (done_time - process_time);
            sts->counter = total_h_customers;
        }

        if (strcmp(pargs->seller_type, "M") == 0)
        {
            if (pargs->tid == 1)
            {
                total_m1_customers++;
                sts->counter = total_m1_customers;
                thr_M1 = thr_M1 + process_time;
                tt_M1 = tt_M1 + done_time + pargs->cq->client[customer_idx].arrival_time;
                rt_M1 = rt_M1 + (done_time - process_time);
            }
            if (pargs->tid == 2)
            {
                total_m2_customers++;
                sts->counter = total_m2_customers;
                thr_M2 = thr_M2 + process_time;
                tt_M2 = tt_M2 + done_time + pargs->cq->client[customer_idx].arrival_time;
                rt_M2 = rt_M2 + (done_time - process_time);
            }
            if (pargs->tid == 3)
            {
                total_m3_customers++;
                sts->counter = total_m3_customers;
                thr_M3 = thr_M3 + process_time;
                tt_M3 = tt_M3 + done_time + pargs->cq->client[customer_idx].arrival_time;
                rt_M3 = rt_M3 + (done_time - process_time);
            }
        }

        if (strcmp(pargs->seller_type, "L") == 0)
        {
            if (pargs->tid == 1)
            {
                total_l1_customers++;
                //printf("\n %d ********************************\n", total_l1_customers);
                sts->counter = total_l1_customers;
                thr_L1 = thr_L1 + process_time;
                tt_L1 = tt_L1 + done_time + pargs->cq->client[customer_idx].arrival_time;
                rt_L1 = rt_L1 + (done_time - process_time);
            }
            if (pargs->tid == 2)
            {
                total_l2_customers++;
                sts->counter = total_l2_customers;
                thr_L2 = thr_L2 + process_time;
                tt_L2 = tt_L2 + done_time + pargs->cq->client[customer_idx].arrival_time;
                rt_L2 = rt_L2 + (done_time - process_time);
            }
            if (pargs->tid == 3)
            {
                total_l3_customers++;
                sts->counter = total_l3_customers;
                thr_L3 = thr_L3 + process_time;
                tt_L3 = tt_L3 + done_time + pargs->cq->client[customer_idx].arrival_time;
                rt_L3 = rt_L3 + (done_time - process_time);
            }
            if (pargs->tid == 4)
            {
                total_l4_customers++;
                sts->counter = total_l4_customers;
                thr_L4 = thr_L4 + process_time;
                tt_L4 = tt_L4 + done_time + pargs->cq->client[customer_idx].arrival_time;
                rt_L4 = rt_L4 + (done_time - process_time);
            }
            if (pargs->tid == 5)
            {
                total_l5_customers++;
                sts->counter = total_l5_customers;
                thr_L5 = thr_L5 + process_time;
                tt_L5 = tt_L5 + done_time + pargs->cq->client[customer_idx].arrival_time;
                rt_L5 = rt_L5 + (done_time - process_time);
            }
            if (pargs->tid == 6)
            {
                total_l6_customers++;
                sts->counter = total_l6_customers;
                thr_L6 = thr_L6 + process_time;
                tt_L6 = tt_L6 + done_time + pargs->cq->client[customer_idx].arrival_time;
                rt_L6 = rt_L6 + (done_time - process_time);
            }
        };
        pt++;
    }
    // thread exits
    return NULL;
}

// starts all threads
void start_all_seller_threads()
{
    pthread_mutex_lock(&mutex);
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);
}

// for comparing arrival times
int compare_arrival_times(const void *a, const void *b)
{
    customer *c1 = (customer *)a;
    customer *c2 = (customer *)b;
    int r = c1->arrival_time - c2->arrival_time;
    if (r > 0)
        return 1;
    if (r < 0)
        return -1;
    return r;
}

// generate the customer queues
void setup_customer_queue(int N)
{
    int i, j, arrival_time;
    cQ = (customerQueue *)malloc(sizeof(customerQueue) * TOTAL_QUEUES);

    // for reproducibility
    srand(1);
    for (i = 0; i < TOTAL_QUEUES; ++i)
    {
        cQ[i].client = (customer *)malloc(sizeof(customer) * N);
        for (j = 0; j < N; ++j)
        {
            // generating random arrival times
            arrival_time = rand() % 60;
            cQ[i].client[j].custId = i * N + j; // i*N+j to get a global customer ID
            cQ[i].client[j].arrival_time = arrival_time;
        }
    }

    // sort customer based on arrival times
    for (i = 0; i < TOTAL_QUEUES; ++i)
    {
        qsort((void *)cQ[i].client, N, sizeof(customer), compare_arrival_times);
    }

    // initialize all mutex value
    for (i = 0; i < ROW_SIZE; ++i)
    {
        pthread_mutex_init(&tickets[i].mutex, NULL);
    }
}

// prints customer queue
void show_customer_queue(int N)
{
    int i, j;

    for (i = 0; i < TOTAL_QUEUES; ++i)
    {

        for (j = 0; j < N; ++j)
        {

            printf(" %d:%d\t|", cQ[i].client[j].custId, cQ[i].client[j].arrival_time);
        }
        printf("\n----------------------------------------------");
        printf("\n");
    }
}

// starts simulation
int main(int argc, char *argv[])
{

    pthread_args *pargs;

    initialize_theater();
    initialize_seat_manager();

    printf("Enter the number of customers: ");
    scanf("%d", &N);
    setup_customer_queue(N);
    show_customer_queue(N);

    int i;
    pthread_t tids[10];
    pargs = (pthread_args *)malloc(sizeof(pthread_args));

    // H
    pargs->seller_type = (char *)malloc(strlen("H") + 1);
    memcpy(pargs->seller_type, "H", strlen("H"));
    pargs->row_id = 0;
    pargs->tid = 0;
    pargs->cq = &cQ[0];
    pthread_create(&tids[0], NULL, (void *)seller_thread, (void *)pargs);

    // M
    for (i = 1; i < 4; i++)
    {
        pargs = (pthread_args *)malloc(sizeof(pthread_args));
        pargs->seller_type = (char *)malloc(strlen("M") + 1);
        memcpy(pargs->seller_type, "M", strlen("M"));
        pargs->row_id = i;
        pargs->tid = i;
        pargs->cq = &cQ[i];
        pthread_create(&tids[i], NULL, (void *)seller_thread, (void *)pargs);
    }

    // L
    for (i = 4; i < 10; i++)
    {
        pargs = (pthread_args *)malloc(sizeof(pthread_args));
        pargs->seller_type = (char *)malloc(strlen("L") + 1);
        memcpy(pargs->seller_type, "L", strlen("L"));
        pargs->row_id = i;
        pargs->tid = i;
        pargs->cq = &cQ[i];
        pthread_create(&tids[i], NULL, (void *)seller_thread, (void *)pargs);
    }

    // start simulation clock
    for (i = 0; i < 60; i++)
    {
        start_all_seller_threads();
        usleep(10);
        if (pt)
        {
            show_theater();
            pt = 0;
        }
        serve_time++;
    }

    printf("\n\n After Sale:\n");
    show_theater();

    printf("\n\n Seats Allocated: (H): %d |  (M): %d | (L): %d | Total: %d | Turn away: %d \n", h_customers, m_customers, l_customers, (h_customers + m_customers + l_customers), N * 10 - (h_customers + m_customers + l_customers));
    printf("\n Average Turn around Time (H):  %d  |  Average Response Time (H):  %d  |  Throughput (H):  %d | Customers Served: %d", tt_H == 0 ? 0 : tt_H / total_h_customers, tt_H == 0 ? 0 : rt_H / total_h_customers, tt_H == 0 ? 0 : thr_H / total_h_customers, total_h_customers);
    printf("\n Average Turn around Time (M): %d  |  Average Response Time (M): %d  |  Throughput (M): %d | Customers Served: %d", tt_M1 + tt_M2 + tt_M3 == 0 ? 0 : tt_M1 + tt_M2 + tt_M3 / m_customers, rt_M1 + rt_M2 + rt_M3 == 0 ? 0 : rt_M1 + rt_M2 + rt_M3 / m_customers, thr_M1 + thr_M2 + thr_M3 == 0 ? 0 : thr_M1 + thr_M2 + thr_M3 / m_customers, m_customers);
    printf("\n Average Turn around Time (L): %d  |  Average Response Time (L): %d  |  Throughput (L): %d | Customers Served: %d", tt_L1 + tt_L2 + tt_L3 + tt_L4 + tt_L5 + tt_L6 == 0 ? 0 : tt_L1 + tt_L2 + tt_L3 + tt_L4 + tt_L5 + tt_L6 / l_customers, rt_L1 + rt_L2 + rt_L3 + rt_L4 + rt_L5 + rt_L6 == 0 ? 0 : rt_L1 + rt_L2 + rt_L3 + rt_L4 + rt_L5 + rt_L6 / l_customers, tt_M2 == 0 ? 0 : thr_L1 + thr_L2 + thr_L3 + thr_L4 + thr_L5 + thr_L6 / l_customers, l_customers);

    //printf("\n Average Turn around Time (H):  %d  |  Average Response Time (H):  %d  |  Throughput (H):  %d | Customers Served: %d", tt_H == 0 ? 0 : tt_H / total_h_customers, tt_H == 0 ? 0 : rt_H / total_h_customers, tt_H == 0 ? 0 : thr_H / total_h_customers, total_h_customers);
    //printf("\n Average Turn around Time (M1): %d  |  Average Response Time (M1): %d  |  Throughput (M1): %d | Customers Served: %d", tt_M1 == 0? 0 : tt_M1 / total_m1_customers, tt_M1 == 0 ? 0 : rt_M1 / total_m1_customers, tt_M1 == 0 ? 0 : thr_M1 / total_m1_customers, total_m1_customers);
    //printf("\n Average Turn around Time (M2): %d  |  Average Response Time (M2): %d  |  Throughput (M2): %d | Customers Served: %d",tt_M2 == 0? 0 : tt_M2 / total_m2_customers, tt_M2 == 0 ? 0 : rt_M2 / total_m2_customers, tt_M2 == 0 ? 0 : thr_M2 / total_m2_customers, total_m2_customers);
    //printf("\n Average Turn around Time (M3): %d  |  Average Response Time (M3): %d  |  Throughput (M3): %d | Customers Served: %d",tt_M3 == 0? 0 : tt_M3 / total_m3_customers, tt_M3 == 0 ? 0 : rt_M3 / total_m3_customers, tt_M3 == 0 ? 0 : thr_M3 / total_m3_customers, total_m3_customers);
    //printf("\n Average Turn around Time (L1): %d  |  Average Response Time (L1): %d  |  Throughput (L1): %d | Customers Served: %d", tt_L1 == 0 ? 0 : tt_L1 / total_l1_customers, tt_L1 == 0 ? 0 : rt_L1 / total_l1_customers, tt_L1 == 0 ? 0 : thr_L1 / total_l1_customers, total_l1_customers);
    //printf("\n Average Turn around Time (L2): %d  |  Average Response Time (L2): %d  |  Throughput (L2): %d | Customers Served: %d",tt_L2 == 0 ? 0 : tt_L2 / total_l2_customers, tt_L2 == 0 ? 0 : rt_L2 / total_l2_customers, tt_L2 == 0 ? 0 : thr_L2 / total_l2_customers, total_l2_customers);
    //printf("\n Average Turn around Time (L3): %d  |  Average Response Time (L3): %d  |  Throughput (L3): %d | Customers Served: %d",tt_L3 == 0 ? 0 : tt_L3 / total_l3_customers, tt_L3 == 0 ? 0 : rt_L3 / total_l3_customers, tt_L3 == 0 ? 0 : thr_L3 / total_l3_customers, total_l3_customers);
    //printf("\n Average Turn around Time (L4): %d  |  Average Response Time (L4): %d  |  Throughput (L4): %d | Customers Served: %d",tt_L4 == 0 ? 0: tt_L4 / total_l4_customers, tt_L4 == 0 ? 0 : rt_L4 / total_l4_customers, tt_L4 == 0 ? 0 : thr_L4 / total_l4_customers, total_l4_customers);
    //printf("\n Average Turn around Time (L5): %d  |  Average Response Time (L5): %d  |  Throughput (L5): %d | Customers Served: %d",tt_L5 == 0 ? 0: tt_L5 / total_l5_customers, tt_L5 == 0 ? 0 : rt_L5 / total_l5_customers, tt_L5 == 0 ? 0 : thr_L5 / total_l5_customers, total_l5_customers);
    //printf("\n Average Turn around Time (L6): %d  |  Average Response Time (L6): %d  |  Throughput (L6): %d | Customers Served: %d",tt_L6 == 0 ? 0: tt_L6 / total_l6_customers, tt_L6 == 0 ? 0 : rt_L6 / total_l6_customers, tt_L6 == 0 ? 0 : thr_L6 / total_l6_customers, total_l6_customers);
}