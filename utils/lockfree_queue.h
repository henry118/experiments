#ifndef __LOCKFREE_QUEUE_H__
#define __LOCKFREE_QUEUE_H__

#include <iostream>
#include <stdexcept>

/*
 * This implementation is based on the algorithm discussed in this paper:
 * http://www.research.ibm.com/people/m/michael/podc-1996.pdf
 */
template<class T>
class lockfree_queue {
    struct node {
        T val;
        node * next;
    } *head, *tail;
public:
    lockfree_queue() {
        node * n = new node;
        n->next = NULL;
        head = tail = n;
    }

    ~lockfree_queue() {
        clear();
    }

    void feed(T val) {
        node * newnode = new node;
        newnode->val = val;
        newnode->next = NULL;

        node * oldtail = tail;
        while (1) {
            if (__sync_bool_compare_and_swap(&oldtail->next, NULL, newnode))
                break;
            __sync_bool_compare_and_swap(&tail, oldtail, oldtail->next);
            oldtail = tail;
        }
        __sync_bool_compare_and_swap(&tail, oldtail, newnode);
    }

    T eat() {
        node *oldhead = head, *oldtail = tail;
        while (1) {
            if (oldhead->next == NULL)
                throw std::out_of_range("empty");
            if (oldhead == oldtail)
                __sync_bool_compare_and_swap(&tail, oldtail, oldtail->next);
            else if (__sync_bool_compare_and_swap(&head, oldhead, oldhead->next))
                break;
            oldhead = head, oldtail = tail;
        }
        T tv = oldhead->next->val;
        delete oldhead;
        return tv;
    }

    void clear() {
        try {
            while (1) eat();
        } catch (...) { }
    }
};

#endif //__LOCKFREE_QUEUE_H__
