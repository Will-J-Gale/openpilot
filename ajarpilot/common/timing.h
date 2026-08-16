#include <time.h>
#include <cstdint>

static inline double seconds_since_boot() {
    struct timespec ts;
    clock_gettime(CLOCK_BOOTTIME, &ts);
    return (double)ts.tv_sec + ((double)ts.tv_nsec * 1e-9);
}

static inline double millis_since_boot() {
    struct timespec ts;
    clock_gettime(CLOCK_BOOTTIME, &ts);
    return (ts.tv_sec * 1000.0) + (ts.tv_nsec * 1e-6);
}

static inline long long nanos_since_boot() {
    struct timespec ts;
    clock_gettime(CLOCK_BOOTTIME, &ts);
    return (ts.tv_sec * 1e9) + ts.tv_nsec;
}

static inline double seconds_since_epoch() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (double)ts.tv_sec + ((double)ts.tv_nsec * 1e-9);
}

static inline double millis_since_epoch() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (ts.tv_sec * 1000.0) + (ts.tv_nsec * 1e-6);
}

static inline long long nanos_since_epoch() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (ts.tv_sec * 1e9) + ts.tv_nsec;
}
