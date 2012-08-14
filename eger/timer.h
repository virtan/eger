#ifndef EGER_TIMER_H
#define EGER_TIMER_H

#include <sys/time.h>
#include <stdlib.h>

namespace eger {

    struct microseconds {
        microseconds() : v(0) {}
        microseconds(size_t _v) : v(_v) {}
        microseconds(const microseconds &other) : v(other.v) {}
        operator size_t() { return v; }
        private:
        size_t v;
    };

    class timer;

    struct deadline_ostream {
        deadline_ostream(timer *_tmr) : tmr(_tmr) {}
        timer *tmr;
    };

    class timer {
        public:
        timer() { reset(); }

        void start() { gettimeofday(&start_time, NULL); }

        size_t current() {
            struct timeval now;
            gettimeofday(&now, NULL);
            size_t diff = (((size_t) now.tv_sec) * 1000000) + now.tv_usec - (((size_t) start_time.tv_sec) * 1000000) - start_time.tv_usec;
            return diff;
        }

        size_t stop() {
            previous_intervals += current();
            stopped_once = true;
            return previous_intervals;
        }

        size_t total() {
            return stopped_once ? previous_intervals : current();
        }

        void reset() {
            previous_intervals = 0;
            stopped_once = false;
            start();
        }

        void deadline(microseconds timeout) {
            long long summ_of_microseconds = ((long long) start_time.tv_usec) + timeout;
            lldiv_t res = lldiv(summ_of_microseconds, 1000000);
            deadline_.tv_usec = res.rem;
            deadline_.tv_sec = start_time.tv_sec + res.quot;
        }

        bool deadline() {
            struct timeval now;
            gettimeofday(&now, NULL);
            return now.tv_sec > deadline_.tv_sec ||
                (now.tv_sec == deadline_.tv_sec && now.tv_usec >= deadline_.tv_usec);
        }

        deadline_ostream deadline_description() { return deadline_ostream(this); }

        bool operator<(microseconds timeout) { return current() < timeout; }
        bool operator>(microseconds timeout) { return current() > timeout; }
        bool operator<=(microseconds timeout) { return current() <= timeout; }
        bool operator>=(microseconds timeout) { return current() >= timeout; }

        private:
        friend std::ostream &operator<<(std::ostream &s, deadline_ostream t);
        struct timeval start_time;
        struct timeval deadline_;
        size_t previous_intervals;
        bool stopped_once;
    };

    inline std::ostream &operator<<(std::ostream &s, timer &t) {
        s << t.current();
        return s;
    }

    inline std::ostream &operator<<(std::ostream &s, deadline_ostream t) {
        s << t.tmr->current() << "ms >= ";
        size_t diff = (((size_t) t.tmr->deadline_.tv_sec) * 1000000) + t.tmr->deadline_.tv_usec - (((size_t) t.tmr->start_time.tv_sec) * 1000000) - t.tmr->start_time.tv_usec;
        s << diff << "ms";
        return s;
    }

}

#endif
