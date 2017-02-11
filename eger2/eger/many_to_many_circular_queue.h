#ifndef MANY_TO_MANY_CIRCULAR_QUEUE_H
#define MANY_TO_MANY_CIRCULAR_QUEUE_H

#include <vector>
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <utility>
#include <math.h>
#include <assert.h>

#include <eger_useful.h>

namespace eger {
    template<class Value>
    class many_to_many_circular_queue {
        private:
            std::vector<Value*> circular_buffer;
            size_t reader_position;
            size_t writer_position;
            size_t requested_size;
            std::mutex exclusive_access;
            std::condition_variable cv;

        public:

            // requires calling init() first
            many_to_many_circular_queue() :
                reader_position(0),
                writer_position(0),
                requested_size(0)
            {}

            // deletes all objects owned
            ~many_to_many_circular_queue() {
                deinit();
            }

            // sets size and resets both positions
            void init(size_t size = 4096) {
                reader_position = 0;
                writer_position = 0;
                requested_size = nearest_power_of_two(size);
            }

            // deletes all objects owned
            void deinit() {
                std::lock_guard<std::mutex> lock(exclusive_access);
                size_t mask = circular_buffer.size() - 1;
                while(likely(reader_position < writer_position))
                    delete circular_buffer[reader_position++ & mask];
            }

            // can return, be set and referenced
            size_t &size() {
                return requested_size;
            }

            // if in capacity — stores ptr and takes ownership, returns true
            // if out of capacity — deletes prev element (to signal overflow), returns false
            bool push(Value *ptr) {
                Value *prev = 0;
                for(;;) {
                    std::lock_guard<std::mutex> lock(exclusive_access);
                    if(unlikely(requested_size != circular_buffer.size())) {
                        requested_size = nearest_power_of_two(requested_size);
                        if(writer_position - reader_position <= requested_size) {
                            // can change the size
                            change_size();
                        } else {
                            size_t circular_offset = get_circular_offset(writer_position - 1);
                            std::swap(prev, circular_buffer[circular_offset]);
                            break;
                        }
                    }
                    if(unlikely(writer_position - reader_position == circular_buffer.size())) { // out of capacity
                        size_t circular_offset = get_circular_offset(writer_position - 1);
                        std::swap(prev, circular_buffer[circular_offset]);
                        break;
                    }
                    size_t circular_offset = get_circular_offset(writer_position);
                    circular_buffer[circular_offset] = ptr;
                    if(reader_position == writer_position++) cv.notify_one();
                    return true;
                }
                delete prev;
                return false;
            }

            // pop blocks until there is a data available
            Value *pop() {
                Value *ptr = 0;
                std::unique_lock<std::mutex> lock(exclusive_access);
                if(unlikely(reader_position == writer_position)) // no data, need to wait
                    cv.wait(lock, [this] { return reader_position < writer_position; });
                size_t circular_offset = get_circular_offset(reader_position++);
                std::swap(ptr, circular_buffer[circular_offset]);
                lock.unlock();
                return ptr;
            }

            // non-blocking check
            bool empty() {
                std::lock_guard<std::mutex> lock(exclusive_access);
                return reader_position == writer_position;
            }

            // atomically checks for emptyness and returns true if empty
            // or returns false and popped value, non-blocking
            std::pair<bool, Value*> empty_or_pop() {
                std::pair<bool, Value*> ret{true, 0};
                std::lock_guard<std::mutex> lock(exclusive_access);
                if(reader_position == writer_position) // no data, don't need to wait
                    return ret;
                ret.first = false;
                size_t circular_offset = get_circular_offset(reader_position++);
                std::swap(ret.second, circular_buffer[circular_offset]);
                return ret;
            }

            // gets everything from buffer
            // blocks if empty
            void pop_all(std::vector<Value*> &dest) {
                std::unique_lock<std::mutex> lock(exclusive_access);
                while(unlikely(dest.size() != circular_buffer.size())) {
                    size_t circular_buffer_size = circular_buffer.size();
                    lock.unlock();
                    dest.resize(circular_buffer_size);
                    lock.lock();
                }
                dest.swap(circular_buffer);
                size_t local_reader_position = reader_position;
                size_t local_writer_position = writer_position;
                reader_position = writer_position;
                lock.unlock();
                // linearize
            }

            // gets everything from buffer
            // doesn't block, returns false if empty
            bool pop_all_nb(std::vector<Value*> &dest) {
            }

        private:

            inline size_t get_circular_offset(size_t absolute_offset) {
                size_t circular_buffer_size = circular_buffer.size();
                return absolute_offset & (circular_buffer_size - 1);
            }

            inline size_t nearest_power_of_two(size_t size) {
                if(size < 2) return 2;
                return pow(2, ceil(log(size)/log(2)));
            }

            void change_size() {
                if(!circular_buffer.size()) {
                    circular_buffer.resize(requested_size);
                    return;
                }
                std::vector<Value*> new_circular_buffer(requested_size);
                size_t mask = circular_buffer.size() - 1;
                size_t new_offset = 0;
                while(reader_position < writer_position)
                    new_circular_buffer[new_offset++] = circular_buffer[reader_position++ & mask];
                reader_position = (((std::max((reader_position - new_offset), (size_t) 1) - 1) // round up to
                           / requested_size) + 1) * requested_size; // multiple of requested_size
                writer_position = reader_position + new_offset;
                circular_buffer.swap(new_circular_buffer);
            }

#ifndef NTEST // many_to_many_circular_queue<int>
        public:

            static bool test_basic_functionality() {
                std::vector<int*> values{new int(1), new int(2)};
                many_to_many_circular_queue<int> target;
                target.init(2);

                assert(target.reader_position == 0);
                assert(target.writer_position == 0);
                bool rc = target.empty();
                assert(rc == true);
                rc = target.push(values[0]);
                assert(rc == true);
                rc = target.empty();
                assert(rc == false);
                assert(target.reader_position == 0);
                assert(target.writer_position == 1);
                int *rv = target.pop();
                assert(rv && *rv == 1);
                rc = target.empty();
                assert(rc == true);
                assert(target.reader_position == 1);
                assert(target.writer_position == 1);

                rc = target.push(values[0]);
                assert(rc == true);
                rc = target.push(values[1]);
                assert(rc == true);
                assert(target.reader_position == 1);
                assert(target.writer_position == 3);
                rc = target.empty();
                assert(rc == false);
                rv = target.pop();
                assert(rv && *rv == 1);
                rv = target.pop();
                assert(rv && *rv == 2);
                rc = target.empty();
                assert(rc == true);
                assert(target.reader_position == 3);
                assert(target.writer_position == 3);

                delete values[0];
                delete values[1];
            }

            static bool test_overfill() {
                std::vector<int*> values{new int(1), new int(2), new int(3)};
                many_to_many_circular_queue<int> target;
                target.init(2);

                bool rc = target.push(values[0]);
                assert(rc == true);
                rc = target.push(values[1]);
                assert(rc == true);
                rc = target.push(values[2]);
                assert(rc == false); // no space, 1 is deleted
                rc = target.empty();
                assert(rc == false); // still have a value
                int* rv = target.pop();
                assert(rv && *rv == 1);
                rc = target.empty();
                assert(rc == false);
                rv = target.pop();
                assert(rv == 0); // a sign of hole
                rc = target.empty();
                assert(rc == true);

                rc = target.push(values[0]);
                assert(rc == true);
                rv = target.pop();
                assert(rv && *rv == 1);

                delete values[0];
                delete values[2];
            }

            static bool test_resize_up() {
                std::vector<int*> values{new int(1), new int(2), new int(3), new int(4)};
                many_to_many_circular_queue<int> target;
                target.init(2);

                target.push(values[0]);
                target.pop();

                target.push(values[0]);
                target.push(values[1]);
                assert(target.reader_position == 1);
                assert(target.writer_position == 3);
                bool rc = target.empty();
                assert(rc == false);

                target.size() = 3; // should be rounded to 4
                assert(target.requested_size == 3);
                rc = target.empty();
                assert(rc == false);
                target.push(values[2]);
                assert(target.requested_size == 4);
                assert(target.reader_position == 4);
                assert(target.writer_position == 7);
                target.push(values[3]);

                int *rv = target.pop();
                assert(rv && *rv == 1);
                rv = target.pop();
                assert(rv && *rv == 2);
                rv = target.pop();
                assert(rv && *rv == 3);
                rv = target.pop();
                assert(rv && *rv == 4);
                rc = target.empty();
                assert(rc == true);
                assert(target.reader_position == 8);
                assert(target.writer_position == 8);

                delete values[0];
                delete values[1];
                delete values[2];
                delete values[3];
            }

            static bool test_resize_down() {
                std::vector<int*> values{new int(1), new int(2), new int(3), new int(4)};
                many_to_many_circular_queue<int> target;
                target.init(4);

                target.push(values[0]);
                target.push(values[1]);
                target.push(values[2]);
                target.size() = 2; // cannot shrink immediately
                bool rc = target.push(values[3]);
                assert(rc == false);
                int *rv = target.pop();
                assert(rv && *rv == 1);
                rc = target.push(values[3]);
                assert(rc == false); // still have no space
                rv = target.pop();
                assert(rv && *rv == 2);
                rc = target.push(values[3]);
                assert(rc == true); // still have no space
                rv = target.pop();
                assert(rv == 0); // sign of overfill
                rv = target.pop();
                assert(rv && *rv == 4);
                assert(target.reader_position == 5);
                assert(target.writer_position == 5);

                delete values[0];
                delete values[1];
                delete values[3];
            }

            static bool test_destructor() {
                std::vector<int*> values{new int(1), new int(2)};
                many_to_many_circular_queue<int> target;
                target.init(2);
                target.push(values[0]);
                target.push(values[1]);
            }
#endif
    };
}

#endif
