#ifndef DCC_A1_UTILS_H
#define DCC_A1_UTILS_H

#include <time.h>

#define NS_PER_MSECOND 1000 * 1000
#define NS_PER_SECOND NS_PER_MSECOND * 1000

#define MIN(x, y) ((x) < (y) ? (x) : (y))

static inline unsigned long ts_diff_in_ns(
        struct timespec * restrict t2,
        struct timespec * restrict t1)
{
    struct timespec diff;

    if (t2->tv_nsec - t1->tv_nsec < 0) {
        diff.tv_sec  = t2->tv_sec  - t1->tv_sec - 1;
        diff.tv_nsec = NS_PER_SECOND + t2->tv_nsec - t1->tv_nsec;
    } else {
        diff.tv_sec  = t2->tv_sec  - t1->tv_sec;
        diff.tv_nsec = t2->tv_nsec - t1->tv_nsec;
    }

    return (diff.tv_sec * NS_PER_SECOND + diff.tv_nsec);
}

#endif /* DCC_A1_UTILS_H */
