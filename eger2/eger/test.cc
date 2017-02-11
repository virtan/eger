#include <iostream>
#include <many_to_many_circular_queue.h>

int main() {
    std::cout << eger::many_to_many_circular_queue<int>::test_basic_functionality() << std::endl;
    std::cout << eger::many_to_many_circular_queue<int>::test_overfill() << std::endl;
    std::cout << eger::many_to_many_circular_queue<int>::test_resize_up() << std::endl;
    std::cout << eger::many_to_many_circular_queue<int>::test_resize_down() << std::endl;
    std::cout << eger::many_to_many_circular_queue<int>::test_destructor() << std::endl;
}
