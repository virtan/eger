#include <eger/useful.h>
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include <chrono>
#include <iomanip>

bool autodetect_ansi(int fd) {
    return isatty(fd);
}

std::ostringstream &current_time_to_stream(std::ostringstream &out) {
    size_t now = now_us();
    time_t t = now/1000000;
    size_t fractional_seconds = (now % 1000000) / 1000;
    out << std::put_time(std::localtime(&t), "%F %T")
        << '.' << std::setw(3) << std::setfill('0') << fractional_seconds;
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
    pthread_t this_thread = pthread_self();
    int policy;
    struct sched_param params;
    [[gnu::unused]] int ret = pthread_getschedparam(this_thread, &policy, &params);
    assert(ret == 0);
    params.sched_priority = std::max(params.sched_priority - 1, 0);
    ret = pthread_setschedparam(this_thread, policy, &params);
    assert(ret == 0);
}

std::string human_readable_number(double d) {
    std::ostringstream os;
    bool minus = false;
    if(d < 0) { os << '-'; d *= -1; minus = true; }
    if(d < 0.0001) { os << "0.0001"; }
    else os << std::setprecision(4) << d;
    std::string r = os.str();
    return d < 0.1 ? (r.size() > (minus ? 7 : 6) ? r.substr(0, (minus ? 7 : 6)) : r) : r;
}
