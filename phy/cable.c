#define _GNU_SOURCE

#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>

#include "cable.h"
#include "../utils.h"

static struct timespec sleep_per_bit;
static unsigned long th_clocks_per_bit;

struct {
    volatile int running;
    volatile int should_stop;
    volatile int lane_high;
    unsigned long clock;

    pthread_t th;
    pthread_cond_t cond;
    pthread_mutex_t mtx;
} cable_th;

static void *cable_th_fn(void *arg __attribute__((unused)))
{
    while (!cable_th.should_stop) {
        cable_th.running = 1;

        /* Check and do work */
        while (!cable_th.should_stop && cable_th.lane_high)
            cable_th.clock++;

        pthread_mutex_lock(&cable_th.mtx);
        while (!cable_th.should_stop && !cable_th.lane_high) {
            /* Signal that we are waiting */
            cable_th.running = 0;
            pthread_cond_signal(&cable_th.cond);

            /* Wait for command */
            pthread_cond_wait(&cable_th.cond, &cable_th.mtx);
        }
        pthread_mutex_unlock(&cable_th.mtx);
    }

    cable_th.running = 0;
    return NULL;
}

static void reset_th_clock(void)
{
    cable_th.clock = 0;
}

/* Not defined as static to be able to use it in tester, meh */
unsigned long get_th_clock(void)
{
    return cable_th.clock;
}

static void wait_th_sleep(void)
{
    pthread_mutex_lock(&cable_th.mtx);
    /* Wait for thread to sleep */
    while (cable_th.running)
        pthread_cond_wait(&cable_th.cond, &cable_th.mtx);
    pthread_mutex_unlock(&cable_th.mtx);
}

static void busy_wait_th_sleep(void)
{
    /* Wait for thread to sleep */
    while (cable_th.running);
}

static void busy_wait_th_run(void)
{
    /* Wait for thread to run */
    while (!cable_th.running);
}

static void set_lane_high(void)
{
    if (!cable_th.lane_high) {
        pthread_mutex_lock(&cable_th.mtx);
        cable_th.lane_high = 1;
        /* Wake up thread */
        pthread_cond_signal(&cable_th.cond);
        pthread_mutex_unlock(&cable_th.mtx);
    }
}

static void set_lane_low(void)
{
    cable_th.lane_high = 0;
}

static int start_th(void)
{
    pthread_cond_init(&cable_th.cond, NULL);
    pthread_mutex_init(&cable_th.mtx, NULL);

    reset_th_clock();
    cable_th.should_stop = 0;
    cable_th.lane_high = 0;
    cable_th.running = 1;
    pthread_create(&cable_th.th, NULL, cable_th_fn, NULL);

    /* Wait for thread init (for it to go to sleep) */
    wait_th_sleep();

    return 0;
}

static void stop_th(void)
{
    pthread_mutex_lock(&cable_th.mtx);
    cable_th.should_stop = 1;
    cable_th.lane_high = 0;
    pthread_cond_signal(&cable_th.cond);
    pthread_mutex_unlock(&cable_th.mtx);

    pthread_join(cable_th.th, NULL);

    pthread_cond_destroy(&cable_th.cond);
    pthread_mutex_destroy(&cable_th.mtx);
}

void init_cable(unsigned long sleep_per_bit_ns,
        unsigned long th_clocks_per_bit_)
{
    sleep_per_bit.tv_sec = 0;
    sleep_per_bit.tv_nsec = sleep_per_bit_ns;

    th_clocks_per_bit = th_clocks_per_bit_;

    start_th();
}

void uninit_cable(void)
{
    stop_th();
}

/* Sleeps for the duration of the transmission of a bit */
static int bit_sleep(void)
{
    struct timespec slp, rem;

    slp = sleep_per_bit;
    while (nanosleep(&slp, &rem)) {
        if (errno != EINTR)
            return -1;
        slp = rem;
    }

    return 0;
}

int send_bit(char bit)
{
    int ret;

    if (bit)
        set_lane_high();
    else
        set_lane_low();

    ret = bit_sleep();
    if (ret)
        return ret;

    return 0;
}

int recv_bit(void)
{
    int ret;
    unsigned long elapsed;

    reset_th_clock();
    set_lane_high();

    ret = bit_sleep();
    if (ret)
        return ret;

    set_lane_low();
    wait_th_sleep();

    elapsed = get_th_clock();
    //printf("diff %lu %lu\n", elapsed, th_clocks_per_bit);
    if (elapsed < th_clocks_per_bit)
        return 1;

    return 0;
}

int is_lane_high(void)
{
    return recv_bit() == 1;
}
