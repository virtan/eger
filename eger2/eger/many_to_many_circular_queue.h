#ifndef MANY_TO_MANY_CIRCULAR_QUEUE_H
#define MANY_TO_MANY_CIRCULAR_QUEUE_H

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
            many_to_many_circular_queue(size_t size) {
                init(size);
            }
            ~many_to_many_circular_queue() {
                deinit();
            }
            void init(size_t size) {
                requested_size = nearest_power_of_two(size);
                reader_position = 0;
                writer_position = 0;
            }
            void deinit() {
            }

            // if in capacity — stores ptr and takes ownership, returns true
            // if out of capacity — deletes ptr, deletes prev element (to signal overflow), returns false
            bool push(Value *ptr) {
                Value *prev = 0;
                for(;;) {
                    std::lock_guard<std::mutex> lock(exclusive_access);
                    size_t circular_offset = get_circular_offset(writer_position - 1);
                    if(unlikely(requested_size != circular_buffer.size())) {
                        if(requested_size > circular_buffer.size()
                                || writer_position - reader_position <= requested_size) {
                            // can change the size
                            std::vector<Value*> new_circular_buffer(requested_size);
                            size_t mask = circular_buffer.size() - 1;
                            while(reader_position < writer_position)
                            if(reader_position != writer_position) {
                                size_t circular_reader_position = get_circular_offset(reader_position);
                                size_t circular_writer_position = get_circular_offset(writer_position);
                                size_t back_amount = circular_buffer.size() - circular_reader_position;
                                memcpy(&new_circular_buffer[0], &circular_buffer[circular_reader_position], circular_buffer.size() - circular_reader_position);
                            }

                        } else {
                            std::swap(prev, circular_buffer[circular_offset]);
                            break;
                        }
                    }
                    if(writer_position - reader_position == circular_buffer.size()) { // out of capacity
                        std::swap(prev, circular_buffer[circular_offset]);
                        break;
                    }
                    circular_buffer[circular_offset] = ptr;
                    if(reader_position == writer_position++) cv.notify_one();
                    return true;
                }
                delete ptr;
                delete prev;
                return false;
            }

            // pop blocks until there is a data available
            Value *pop() {
                Value *ptr = 0;
                std::unique_lock<std::mutex> lock(exclusive_access);
                if(reader_position == writer_position) // no data, need to wait
                    cv.wait(lock, [] { return reader_position < writer_position; });
                size_t circular_offset = get_circular_offset(reader_position++);
                std::swap(ptr, circular_buffer[circular_offset]);
                lock.unlock();
                return ptr;
            }

            // non-blocking
            bool empty() const {
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

        private:

            inline size_t get_circular_offset(size_t absolute_offset) {
                circular_buffer_size = circular_buffer.size();
                return absolute_offset & (circular_buffer_size - 1);
            }

            inline size_t nearest_power_of_two(size_t size) {
                if(size < 2) return 2;
                return pow(2, ceil(log(size)/log(2)));
            }

    };
}

#endif
