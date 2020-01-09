#define _GNU_SOURCE

#include <stdio.h>
#include <errno.h>
#include <time.h>

#include "cable.h"
#include "../utils.h"

unsigned long keep_lane_high(unsigned long time_ns)
{
    unsigned long clock = 0;
    struct timespec start_time, time;

    clock_gettime(CLOCK_MONOTONIC_RAW, &start_time);
    for (;;) {
        clock++;

        clock_gettime(CLOCK_MONOTONIC_RAW, &time);
        if (time_ns <= ts_diff_in_ns(&time, &start_time))
            break;
    }

    return clock;
}

int keep_lane_low(unsigned long time_ns)
{
    struct timespec rem;
    struct timespec slp = {
        .tv_sec = 0,
        .tv_nsec = time_ns
    };

    if (time_ns >= NS_PER_SECOND)
        return -1;

    while (nanosleep(&slp, &rem)) {
        if (errno != EINTR)
            return -1;
        slp = rem;
    }

    return 0;
}
