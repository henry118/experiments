#include <iostream>

class atomic {
    volatile int val;
public:
    atomic(int v = 0) : val(v) {}
    operator int() {
        return __sync_val_compare_and_swap(&val, 0, 0);
    }
    int operator++() {
        return __sync_add_and_fetch(&val, 1);
    }
    int operator--() {
        return __sync_sub_and_fetch(&val, 1);
    }
    int operator++(int) {
        return __sync_fetch_and_add(&val, 1);
    }
    int operator--(int) {
        return __sync_fetch_and_sub(&val, 1);
    }
    int operator+=(int v) {
        return __sync_add_and_fetch(&val, v);
    }
    int operator-=(int v) {
        return __sync_sub_and_fetch(&val, v);
    }
    bool operator==(int v) {
        return __sync_bool_compare_and_swap(&val, v, v);
    }
};

int main(int argc, char * argv[]) {
    atomic v;
    if ((v += 9) == 9) {
        std::cout << "yes" << std::endl;
    }
    return 0;
}
