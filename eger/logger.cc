#include <iostream>
#include "logger.h"
#include "writer.h"

namespace eger {

instance *instance::eger_instance_ = 0;
writer *instance::wrt = 0;
std::thread *instance::wrt_thread = 0;

instance::instance() :
    maximum_log_size(20*1024*1024),
    ansi_colors(true)
{
    eger_instance_ = this;
    wrt = 0;
    wrt_thread = 0;
    resize(log_level_size);
}

instance::~instance() {
    if(wrt) wrt->stop();
    if(wrt_thread) wrt_thread->join();
    delete wrt_thread;
    delete wrt;
    wrt_thread = 0;
    wrt = 0;
    eger_instance_ = 0;
}

void instance::start_writer() {
    // born thread with instance of writer
    assert(!wrt && !wrt_thread);
    wrt = new writer(this, 64*1024);
    wrt_thread = new std::thread(writer::static_run, wrt);
}

void instance::start_sync_writer() {
    assert(!wrt && !wrt_thread);
    wrt = new writer(this);
}

void instance::pass_to_writer(log_stream *ls) {
    if(!wrt) {
        log_stream els(level_warning);
        els << "eger::writer hasn't been started";
        std::cerr << writer::compose_log_string(&els, ansi_colors);
        delete ls;
        return;
    }
    wrt->push_back(ls);
}


}
