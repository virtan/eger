#ifndef EGER_WRITER_H
#define EGER_WRITER_H

#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <algorithm>
#include <iterator>
#include <chrono>
#include <eger/types.h>
#include <eger/logger.h>

namespace eger {

class writer {
    public:
    writer(instance *_inst, size_t queue_size); // for asynchronous writer
    writer(instance *_inst); //for synchronous writer
    ~writer();

    static void static_run(writer *wrt);

    void push_back(log_stream *ls);
    void stop();

    static string compose_log_string(log_stream *ls, bool ansi_colors = true);
    static const char *level_to_string(log_level l, bool ansi_colors);

    private:
    void perform_writing(log_stream *ls, int &fd);
    void write_logs();
    bool try_open_file(const string &fn, int &fd);
    void check_size_and_rename(const string &fn);
    void rename_log(const string &from, const string &to);
    void run();
    inline size_t next_nearest_power_of_2(size_t v);

    private:
    friend class instance;
    instance *inst;
    log_stream **queue;
    size_t queue_mask;
    size_t writing;
    size_t accepting;
    bool wait_for_finish;
    bool finished;
    bool sync_mode;
};

}

#endif
