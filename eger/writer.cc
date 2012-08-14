#include "writer.h"

namespace eger {

writer::writer(instance *_inst, size_t queue_size) :
    inst(_inst),
    writing(0),
    accepting(0),
    wait_for_finish(false),
    finished(false),
    sync_mode(false)
{
    assert(queue_size > 0);
    queue_size = next_nearest_power_of_2(queue_size);
    queue = new log_stream*[queue_size];
    queue_mask = queue_size - 1;
    memset(queue, 0, queue_size * sizeof(log_stream*));
}

writer::writer(instance *_inst) :
    inst(_inst),
    queue(0),
    queue_mask(0),
    writing(0),
    accepting(0),
    wait_for_finish(false),
    finished(false),
    sync_mode(true)
{}


writer::~writer() { delete[] queue; }

void writer::static_run(writer *wrt) { wrt->run(); }

void writer::push_back(log_stream *ls) {
    if(sync_mode) {
        int fd = -1;
        perform_writing(ls, fd);
        if(fd >= 0) close(fd);
        return;
    }
    size_t my_place = __sync_fetch_and_add(&accepting, 1);
    my_place &= queue_mask;
    if(queue[my_place]) {
        log_stream els(level_warning);
        els << "log queue full, dropping record";
        std::cerr << compose_log_string(&els, inst->ansi_colors);
        delete ls;
    } else {
        queue[my_place] = ls;
    }
}

void writer::stop() {
    wait_for_finish = true;
    while(!sync_mode && !finished)
        usleep(100000);
}

string writer::compose_log_string(log_stream *ls, bool ansi_colors) {
    using namespace boost::posix_time;
    using namespace std;
    ostringstream d;
    struct tm now_t = to_tm(ls->moment);
    if(ansi_colors) d << char(27) << "[38;5;238m";
    /* no date
    d << (now_t.tm_year + 1900) << '.' << setw(2) << setfill('0') << (now_t.tm_mon + 1) << '.' <<
        setw(2) << setfill('0') << now_t.tm_mday;
    d << ' ';
    */
    d << setw(2) << setfill('0') << now_t.tm_hour << ':' <<
        setw(2) << setfill('0') << now_t.tm_min << ':' <<
        setw(2) << setfill('0') << now_t.tm_sec;
    d << '.' << setw(3) << setfill('0') << (ls->moment.time_of_day().fractional_seconds() / 1000);
    d << ' ';
    d << level_to_string(ls->lvl, ansi_colors);
    d << ' ';
    if(ansi_colors) d << char(27) << "[m";
    string ls_str(ls->str());
    transform(ls_str.begin(), ls_str.end(), ostream_iterator<uint8_t>(d),
            [=](const uint8_t c) -> uint8_t { if(!ls->multiline && c < ' ') return ' '; else return c; });
    d << '\n';
    return d.str();
}

const char *writer::level_to_string(log_level l, bool ansi_colors) {
    switch(l) {
        case level_critical:   return ansi_colors ? "\x1b[38;5;124mcritical    " : "critical    ";
        case level_error:      return ansi_colors ? "\x1b[38;5;124merror       " : "error       ";
        case level_warning:    return ansi_colors ? "\x1b[38;5;184mwarning     " : "warning     ";
        case level_info:       return ansi_colors ? "\x1b[38;5;112minfo        " : "info        ";
        case level_profile:    return ansi_colors ? "\x1b[38;5;135mprofile     " : "profile     ";
        case level_debug:      return ansi_colors ? "\x1b[38;5;130mdebug       " : "debug       ";
        case level_debug_hard: return ansi_colors ? "\x1b[38;5;131mdebug_hard  " : "debug_hard  ";
        case level_debug_mare: return ansi_colors ? "\x1b[38;5;133mdebug_mare  " : "debug_mare  ";
    }
    return "unknown";
}

void writer::perform_writing(log_stream *ls, int &fd) {
    string &file_name = (*inst)[(size_t) ls->lvl];
    bool print_to_file = file_name != "stdout" && file_name != "stderr";
    if(print_to_file && fd == -1)
        if(!try_open_file(file_name, fd)) {
            std::cerr << compose_log_string(ls, inst->ansi_colors);
            delete ls;
            return;
        }
    string log_string = compose_log_string(ls, inst->ansi_colors);
    int rfd = print_to_file ? fd : file_name[5] == 't' ? 1 : 2;
    write(rfd, log_string.data(), log_string.size());
    delete ls;
}

void writer::write_logs() {
    std::vector<int> fds;
    for(size_t i = 0; i < log_level_size; ++i)
        fds.push_back(-1);
    size_t lost_record = size_t(0) - 1;
    for(; writing < accepting;
            __sync_synchronize(), ++writing) {
        size_t offset = writing & queue_mask;
        log_stream *ls = __sync_fetch_and_and(queue + offset, 0);
        if(!ls) {
            if(lost_record == size_t(0) - 1) lost_record = writing;
            continue;
        }
        perform_writing(ls, fds[(size_t) ls->lvl]);
    }
    if(lost_record != size_t(0) - 1)
        writing = lost_record;
    for(size_t i = 0; i < log_level_size; ++i)
        if(fds[i] != -1) close(fds[i]);
}

bool writer::try_open_file(const string &fn, int &fd) {
    check_size_and_rename(fn);
    fd = open(fn.c_str(), O_WRONLY | O_APPEND | O_CREAT, 0660);
    if(fd < 0) {
        log_stream ls(level_warning);
        char str_buf[256];
        strerror_r(errno, str_buf, 256);
        ls << "can't open file \"" << fn << "\" for writing: " << str_buf;
        std::cerr << compose_log_string(&ls, inst->ansi_colors);
        return false;
    }
    return true;
}

void writer::check_size_and_rename(const string &fn) {
    struct stat fn_stat;
    if(stat(fn.c_str(), &fn_stat) < 0) return;
    if((size_t) fn_stat.st_size > inst->maximum_log_size)
        rename_log(fn, fn + ".0");
}

void writer::rename_log(const string &from, const string &to) {
    if(access(to.c_str(), F_OK) == 0) {
        // exists
        string::size_type ind = to.rfind('.');
        if(ind != string::npos) {
            ++ind;
            long ind_n = atol(to.c_str() + ind);
            std::ostringstream next_to;
            next_to << to.substr(0, ind);
            next_to << (ind_n + 1);
            rename_log(to, next_to.str());
        }
    }
    if(rename(from.c_str(), to.c_str()) < 0) {
        log_stream ls(level_warning);
        char str_buf[256];
        strerror_r(errno, str_buf, 256);
        ls << "can't rename file \"" << from << "\" to \"" << to << "\": " << str_buf;
        std::cerr << compose_log_string(&ls, inst->ansi_colors);
    }
}

void writer::run() {
    using namespace boost::posix_time;
    while(true) {
        ptime start_time = microsec_clock::local_time();
        __sync_synchronize();
        if(writing < accepting) write_logs();
        __sync_synchronize();
        if(wait_for_finish && writing == accepting) {
            finished = true;
            break;
        }
        ptime next_cycle_time = start_time + seconds(1);
        ptime now = microsec_clock::local_time();
        if(next_cycle_time > now) {
            if(wait_for_finish) usleep((next_cycle_time - now).total_microseconds() / 10);
            else usleep((next_cycle_time - now).total_microseconds());
        }
    }
}

inline size_t writer::next_nearest_power_of_2(size_t v) {
    --v;
    for(size_t i = 1; i < sizeof(v) * 8; i *= 2)
        v |= v >> i;
    return ++v;
}

}
