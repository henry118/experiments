#ifndef __MEMORYPOOL_H__
#define __MEMORYPOOL_H__

#include <stdint.h>
#include <bitset>
#include <cmath>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <boost/mpl/arithmetic.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>

/*
 * A memory pool using buddy allocation algorithm
 */
template<size_t Unit, size_t Order>
class MemoryPool {
private:
    template<size_t order>
    struct nodes_at_order {
        static const size_t value = (1 << order);
    };

    template<size_t order>
    struct nodes_of_tree {
        static const size_t value = (nodes_at_order<order>::value << 1 - 1);
    };


    template<size_t sz, size_t bound>
    struct align {
        static const size_t value = (sz + bound - 1) & (~(bound - 1));
    };

    template<size_t sz, size_t bound>
    struct is_aligned {
        typedef boost::mpl::bool_<
            boost::is_same<
                typename boost::mpl::modulus<
                    boost::mpl::int_<sz>,
                    boost::mpl::int_<bound>
                    >::type,
                boost::mpl::integral_c<int, 0>
                >::value
            > type;
        static const bool value = type::value;
    };

    static const size_t unit_size = Unit;
    static const size_t order = Order;
    static const size_t total_size = (nodes_at_order<Order>::value * unit_size);
    static const size_t max_nodes = nodes_of_tree<Order>::value;

public:
    MemoryPool() {
        BOOST_STATIC_ASSERT((is_aligned<unit_size, 4>::value));
        chunk = (unsigned char*)malloc(total_size);
        std::cerr << "total size: " << total_size
                  << ", base address: 0x" << std::hex << (uint64_t)chunk
                  << std::endl;
    }

    ~MemoryPool() {
        if (chunk) {
            free(chunk);
            chunk = 0;
        }
    }

    template<size_t N>
    void * allocate() {
        return allocate<N>(typename is_aligned<N, unit_size>::type());
    }

    template<size_t N>
    void deallocate(void * ptr) {
        deallocate<N>(ptr, typename is_aligned<N, unit_size>::type());
    }

private:
    template<size_t N>
    void * allocate(boost::mpl::false_) {
        return allocate<align<N, unit_size> >();
    }

    template<size_t N>
    void deallocate(void * ptr, boost::mpl::false_) {
        deallocate<align<N, unit_size> >(ptr);
    }

    template<size_t N>
    void * allocate(boost::mpl::true_) {
        size_t target_order = order_of_size(N);
        size_t node_index = 0, left_node = 0, right_node = 0;
        while (1) {
            left_node = left_child(node_index);
            right_node = right_child(node_index);
            if (is_freenode(node_index) && order_at_index(node_index) == target_order) {
                set_fullnode(node_index);
                mark_parents(node_index);
                std::cerr << "allocated index: " << node_index << std::endl;
                return index_to_address(node_index);
            } else if (potential(left_node, target_order)) {
                node_index = left_node;
            } else if (potential(right_node, target_order)) {
                node_index = right_node;
            } else {
                break;
            }
        }
        return 0;
    }

    template<size_t N>
    void deallocate(void * ptr, boost::mpl::true_) {
        size_t node_index = address_to_index(ptr, N);
        assert(node_index < max_nodes);
        std::cerr << "deallocating index: " << node_index << std::endl;
        full_flags[node_index] = false;
        partial_flags[node_index] = false;
        mark_parents(node_index);
    }

    void * index_to_address(size_t node_index) {
        size_t n = left_most_leaf(node_index) - left_most_leaf(0);
        void * addr = (chunk + n);
        return addr;
    }

    size_t address_to_index(void * address, size_t bytes) {
        size_t n = (unsigned char*)address - chunk;
        size_t leaf = left_most_leaf(0) + n;
        size_t diff = order - order_of_size(bytes);
        return (leaf + 1) / (1 << diff) - 1;
    }

    static size_t order_of_size(size_t sz) {
        size_t rval = order + 3 - log2(sz);
        std::cerr << "order of size [" << sz << "]: " << rval << std::endl;
        return rval;
    }

    static size_t order_at_index(size_t node_index) {
        size_t rval = log2(node_index + 1);
        std::cerr << "order at index [" << node_index << "]: " << rval << std::endl;
        return rval;
    }

    static size_t parent(size_t node_index) {
        return (node_index - 1) >> 1;
    }

    static size_t left_child(size_t node_index) {
        return (node_index << 1) + 1;
    }

    static size_t right_child(size_t node_index) {
        return (node_index << 1) + 2;
    }

    static size_t sibling(size_t node_index) {
        return (node_index % 2) ? node_index + 1 : node_index - 1;
    }

    size_t left_most_leaf(size_t node_index) {
        size_t diff = order - order_at_index(node_index);
        return (1 << diff) * (node_index + 1) - 1;
    }

    size_t right_most_leaf(size_t node_index) {
        size_t diff = order - order_at_index(node_index);
        return (1 << diff) * (node_index + 2) - 2;
    }

    bool is_freenode(size_t node_index) {
        bool rval = !full_flags[node_index] && !partial_flags[node_index];
        std::cerr << "node[" << node_index << "] is free." << std::endl;
        return rval;
    }

    void set_freenode(size_t node_index) {
        full_flags[node_index] = false;
        partial_flags[node_index] = false;
        std::cerr << "set node [" << node_index << "] free." << std::endl;
    }

    void set_fullnode(size_t node_index) {
        full_flags[node_index] = true;
        partial_flags[node_index] = false;
        std::cerr << "set node [" << node_index << "] full." << std::endl;
    }

    void set_partialnode(size_t node_index) {
        full_flags[node_index] = false;
        partial_flags[node_index] = true;
        std::cerr << "set node [" << node_index << "] partial." << std::endl;
    }

    bool potential(size_t node_index, size_t target_order) {
        if (node_index >= max_nodes || full_flags[node_index])
            return false;
        size_t oati = order_at_index(node_index);
        if (oati > target_order || (oati == target_order && partial_flags[node_index]))
            return false;
        return true;
    }

    void mark_parents(size_t node_index) {
        size_t parent_node, sibling_node;
        while (node_index > 0) {
            parent_node = parent(node_index);
            sibling_node = sibling(node_index);
            if (is_freenode(node_index) && is_freenode(sibling_node))
                set_freenode(parent_node);
            else if (full_flags[node_index] && full_flags[sibling_node])
                set_fullnode(parent_node);
            else
                set_partialnode(parent_node);
            node_index = parent_node;
        }
    }

private:
    unsigned char * chunk; // the pool buffer
    std::bitset<max_nodes> full_flags; // flags indicates the node is full
    std::bitset<max_nodes> partial_flags; // flags indicates the node is partially filled
};

#endif //__MEMORYPOOL_H__
