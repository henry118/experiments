#ifndef __LOCKFREE_RINGBUFFER_H__
#define __LOCKFREE_RINGBUFFER_H__ 

#include <stdexcept>

template<class T, size_t N>
class lockfree_ringbuffer {
    T ring[N];
    size_t head, tail;
public:
    lockfree_ringbuffer() : head(0), tail(0) {}
    void feed(T val) {
        size_t h = head, t = tail;
        while (1) {
            if (h - t >= N)
                throw std::range_error("full");
            if (__sync_bool_compare_and_swap(&head, h, h + 1)) {
                ring[(h+1) % N] = val;
                break;
            }
            h = head, t = tail;
        }
    }
    T eat() {
        size_t h = head, t = tail;
        while (1) {
            if (h == t)
                throw std::range_error("empty");
            if (__sync_bool_compare_and_swap(&tail, t, t + 1)) {
                return ring[(t+1) % N];
            }
            t = head, t = tail;
        }
    }
};

#endif //__LOCKFREE_RINGBUFFER_H__
