#ifndef __REFCNT_H__
#define __REFCNT_H__

class atomic_counter {
    int value;
public:
    explicit atomic_counter(int initval = 0) : value(initval) {}
    atomic_counter(const atomic_counter & other) : value(other.value) {}
    operator int() { return value; }
    atomic_counter & operator++ () {
        __sync_fetch_and_add(&value, 1);
        return *this;
    }
    atomic_counter operator++ (int) {
        return atomic_counter(__sync_fetch_and_add(&value, 1));
    }
    atomic_counter & operator-- () {
        __sync_fetch_and_add(&value, -1);
        return *this;
    }
    atomic_counter operator-- (int) {
        return atomic_counter(__sync_fetch_and_add(&value, -1));
    }
};

template<class T>
class RefT {
    T * ptr;
public:
    explicit RefT(T * val = 0) : ptr(val) {
        if (ptr) {
            ++ptr->ref;
        }
    }
    RefT(const RefT<T> & other) : ptr(other.ptr) {
        if (ptr) {
            ++ptr->ref;
        }
    }
    ~RefT() {
        if (ptr && --ptr->ref == 0) {
            delete ptr;
        }
    }
    operator T * () {
        return ptr;
    }
    T * operator-> () {
        return ptr;
    }
    RefT<T> & operator= (const RefT<T> & other) {
        this->~RefT();
        ptr = other.ptr;
        ++ptr->ref;
        return *this;
    }
    bool operator== (const RefT<T> & other) const {
        return ptr == other.ptr;
    }
    bool operator!= (const RefT<T> & other) const {
        return ptr != other.ptr;
    }

};

#endif //__REFCNT_H__
