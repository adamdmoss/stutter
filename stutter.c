
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#include "timefn.h"

#define INTERVAL_US() 1000
#define USE_CPUTIME() 1
const uint64_t NUM_MS_TO_RUN = 10 * 1000 *9999*3*2UL;

void spinReal1ms()
{
    const UTIL_time_t starttime = UTIL_getTime();
    do
    {
        if (UTIL_clockSpanMicro(starttime) >= INTERVAL_US()) break;
    } while(1);
}

typedef struct { time_t tv_sec; long tv_nsec; } timespec;

void main(void)
{
    //const uint64_t NOTEWORTHY_NS_OVERRUN = 100;

#define bucketcount 20000
#define bucketwidth_us 100
    int overrunbucketsus[bucketcount] = { 0 };
#define NS_TO_BUCKET(ns) ( (ns) / (bucketwidth_us*1000UL) < bucketcount ? (ns) / (bucketwidth_us*1000UL) : bucketcount-1 )

    PTime CPUTimeOverheadNS = -1;
    for (int i=0; i<10000000; ++i)
    {
#if USE_CPUTIME()
        UTIL_waitForNextTick();
        const UTIL_time_t a = UTIL_getTime();
        const PTime OverheadNS = UTIL_getSpanTimeNano(a,UTIL_getTime());
#else
        struct timespec ts = { 0, 0 };
        UTIL_waitForNextTick();
        const UTIL_time_t a = UTIL_getTime();
        //nanosleep(&ts, NULL);
        const PTime OverheadNS = UTIL_getSpanTimeNano(a,UTIL_getTime());
#endif
        if (OverheadNS < CPUTimeOverheadNS && OverheadNS > 0)
            CPUTimeOverheadNS = OverheadNS;
    }
    if (CPUTimeOverheadNS == -1) exit(-1);
    printf("Using CPU timing?%d (outer loop CPU time overhead appears to be %luns)\n", USE_CPUTIME(), CPUTimeOverheadNS);

    uint64_t runs_remaining = (NUM_MS_TO_RUN * 1000) / INTERVAL_US();
    printf("Interval=%dus, NUM_MS_TO_RUN=%ld so num_iters=%ld\n", INTERVAL_US(), NUM_MS_TO_RUN, runs_remaining);
    while(runs_remaining--) {
#if USE_CPUTIME()
        //UTIL_waitForNextTick();
        const UTIL_time_t starttime = UTIL_getTime();
        //spinReal1ms();
        const UTIL_time_t endtime = UTIL_getTime();
        const PTime span = UTIL_getSpanTimeNano(starttime,endtime);
        int64_t overrun_ns = span - 0*1000L*INTERVAL_US() - CPUTimeOverheadNS;
#else
        struct timespec ts = { 0, 1000L * INTERVAL_US() };
        UTIL_waitForNextTick();
        const UTIL_time_t starttime = UTIL_getTime();
        nanosleep(&ts, NULL);
        const UTIL_time_t endtime = UTIL_getTime();
        const PTime span = 2*1000UL*INTERVAL_US()-((UTIL_getSpanTimeNano(starttime,endtime) - CPUTimeOverheadNS));
#endif
        if (//span <= 1000UL*INTERVAL_US() &&
            1){//1000UL*INTERVAL_US()-span >= NOTEWORTHY_NS_OVERRUN) {
            //printf("span=%luns, overrun=%ldns\t", span, overrun_ns);
            if (overrun_ns >= 0)
                overrunbucketsus[NS_TO_BUCKET(overrun_ns)]++;
            else
                {printf("*");overrunbucketsus[NS_TO_BUCKET(0)]++;}
        }
    }

    printf("\n");

    for (int i=0; i<bucketcount; ++i)
    {
        if(overrunbucketsus[i])printf("\n%09dus : %d", bucketwidth_us*(1+i), overrunbucketsus[i]);
    }

    /*
    UTIL_waitForNextTick();
    const UTIL_time_t starttime = UTIL_getCPUTime();
    UTIL_waitForNextTick();
    UTIL_time_t endtime = UTIL_getCPUTime();
    fprintf(stderr, "tick is %luns\n", UTIL_getSpanTimeNano(starttime, endtime));
    sleep(1);//spinReal1ms();
    endtime = UTIL_getCPUTime();
    fprintf(stderr, "overall is %luns\n", UTIL_getSpanTimeNano(starttime, endtime));
    */
    printf("\n");
}

