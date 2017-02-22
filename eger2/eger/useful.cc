#include <eger/useful.h>
#include <sys/time.h>
#include <assert.h>

bool autodetect_ansi() {
    // TODO
    return true;
}

std::ostringstream &date(std::ostringstream &out) {
    return out;
}

bool writev_all(int fd, const struct iovec *iov, int iovcnt) {
    // TODO
    return false;
}

size_t now_us() {
#ifdef __linux
    struct timespec ts;
    int rc = clock_gettime(CLOCK_REALTIME_COARSE, &ts);
    assert(rc == 0);
    return (size_t) ts.tv_sec * 1000000 + (size_t) ts.tv_nsec / 1000;
#else
    struct timeval tv;
    int rc = gettimeofday(&tv, 0);
    assert(rc == 0);
    return (size_t) tv.tv_sec * 1000000 + (size_t) tv.tv_usec;
#endif
}

void lower_this_thread_priority() {
    // TODO
}
