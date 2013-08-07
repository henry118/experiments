#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#define MAX_ORDER 5

/*
           o        64
          / \
        o     o     32
       / \   / \
       o o   o o    16
*/

#define FREE 0
#define PARTIAL 1
#define FULL 2

#define ND_MIN_SZ (1 << 4)
#define ND_NR_AT_ORDER(order) (1 << (order))
#define ND_NR(order) ((ND_NR_AT_ORDER(order) << 1) - 1)
#define POOL_SZ(order) (ND_NR_AT_ORDER(order) * ND_MIN_SZ)
#define MAX_ND_NR ND_NR(MAX_ORDER)

#define IS_POW2(n) ({                           \
            unsigned __n = n;                   \
            !(n & (n-1)); })

#define NEXT_POW2(n) ({                         \
            unsigned __n = (n);                 \
            __n |= __n >> 1;                    \
            __n |= __n >> 2;                    \
            __n |= __n >> 4;                    \
            __n |= __n >> 8;                    \
            __n |= __n >> 16;                   \
            ++__n; })

#define LOG2(n) ({                                      \
    unsigned __r = 0, __n = (n);                        \
    if (__n >= 1<<16) { __n >>= 16; __r |= 16; }        \
    if (__n >= 1<< 8) { __n >>=  8; __r |=  8; }        \
    if (__n >= 1<< 4) { __n >>=  4; __r |=  4; }        \
    if (__n >= 1<< 2) { __n >>=  2; __r |=  2; }        \
    if (__n >= 1<< 1) __r |= 1;                         \
    __r; })

#define PARENT(n) (((n) - 1) >> 1)
#define LEFT(n)   (((n) << 1) + 1)
#define RIGHT(n)  (((n) << 1) + 2)
#define SIBLING(n)(((n) % 2) ? (n)+1 : (n)-1)
#define HEIGHT(n) LOG2((n)+1)
#define LEFT_LEAF(h) ((1 << (h)) - 1)

struct node {
    unsigned char status : 2;
    unsigned char nu: 5;
};

struct mem_pool {
    unsigned char order;
    unsigned char valid_nr;
    struct node nl[MAX_ND_NR];
    unsigned char chunk[1];
};

#define CHUNK_OFFSET offsetof(struct mem_pool, chunk)

struct mem_pool *
mem_pool_new(unsigned order) {
    if (order > MAX_ORDER)
        return NULL;
    size_t sz = POOL_SZ(order) + CHUNK_OFFSET;
    struct mem_pool * p = (struct mem_pool*)malloc(sz);
    bzero(p, CHUNK_OFFSET);
    p->order = order;
    p->valid_nr = ND_NR(order);
    return p;
}

void
mem_pool_destroy(struct mem_pool * pool) {
    if (pool)
        free(pool);
}

static int
potential(struct mem_pool * pool, unsigned index, unsigned order) {
    if (index >= pool->valid_nr) {
        printf("index too big: %d\n", index);
        return 0;
    }
    struct node * n = &pool->nl[index];
    if (n->status == FULL) {
        printf("full\n");
        return 0;
    }
    if (HEIGHT(index) > order) {
        printf("order too big: %d\n", HEIGHT(index));
        return 0;
    }
    if (n->status == PARTIAL && HEIGHT(index) == order) {
        printf ("partial\n");
        return 0;
    }
    printf("node %d has potential!\n", index);
    return 1;
}

static void
mark_parent(struct mem_pool * pool, unsigned index) {
    static const unsigned char map[3][3] = {
        { FREE, PARTIAL, PARTIAL },
        { PARTIAL, PARTIAL, PARTIAL },
        { PARTIAL, PARTIAL, FULL }
    };
    for(;;) {
        if (index == 0)
            break;
        struct node * pn = &pool->nl[PARENT(index)];
        struct node * sn = &pool->nl[SIBLING(index)];
        struct node * in = &pool->nl[index];
        pn->status = map[sn->status][in->status];
        index = PARENT(index);
    }
}

int
mem_pool_alloc(struct mem_pool * pool, size_t size) {
    size_t sz = size >= ND_MIN_SZ ? size : ND_MIN_SZ;
    sz = IS_POW2(sz) ? sz : NEXT_POW2(sz);
    printf("aligning requested size %d to %d...\n", size, sz);

    unsigned order = pool->order + 4 - LOG2(sz);
    printf("requested order is %d\n", order);

    unsigned i = 0, rval = 0;

    if (!potential(pool, i, order)) {
        return -1;
    }

    for (;;) {
        printf("checking node %d ...\n", i);
        struct node * n = &pool->nl[i];
        if (n->status == FREE && HEIGHT(i) == order) {
            n->status = FULL;
            mark_parent(pool, i);
            while (i < pool->valid_nr) {
                rval = i;
                i = LEFT(i);
            }
            return rval - LEFT_LEAF(pool->order); //good
        } else if (potential(pool, LEFT(i), order)) {
            i = LEFT(i);
        } else if (potential(pool, RIGHT(i), order)) {
            i = RIGHT(i);
        } else {
            break;
        }
    }

    return -1;
}

void
mem_pool_free(struct mem_pool * pool, int index) {
    struct node * n;
    index += LEFT_LEAF(pool->order);
    for(;;) {
        n = &pool->nl[index];
        if (n->status == FULL)
            break;
        index = PARENT(index);
    }
    n->status = FREE;
    mark_parent(pool, index);
}

unsigned
block_size(struct mem_pool * pool, int index) {
    struct node * n;
    index += LEFT_LEAF(pool->order);
    for(;;) {
        n = &pool->nl[index];
        if (n->status == FULL)
            break;
        index = PARENT(index);
    }
    int h = HEIGHT(index);
    return POOL_SZ(pool->order - h);
}

static void printb(struct mem_pool * pool, int index) {
    printf("[!] returned index: %d, size: %d\n", index, block_size(pool, index));
}


int main(int argc, char * argv[]) {
    struct mem_pool * pool = mem_pool_new(4);

    int r1,r2,r3,r4;

    r1 = mem_pool_alloc(pool, 16);
    printb(pool, r1);

    r2 = mem_pool_alloc(pool, 32);
    printb(pool, r2);

    r3 = mem_pool_alloc(pool, 16);
    printb(pool, r3);

    mem_pool_free(pool, r1);
    mem_pool_free(pool, r3);

    r4 = mem_pool_alloc(pool, 32);
    printb(pool, r4);

    mem_pool_destroy(pool);
    return 0;
}
